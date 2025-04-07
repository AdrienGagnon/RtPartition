#include "RtMidiHandler.h"
#include "LomseViewWidget.h"
#include "NoteCommand.h"

#include <lomse_presenter.h>
#include <lomse_interactor.h>
#include <lomse_doorway.h>
#include <lomse_command.h>
#include <lomse_interactor.h>
#include <qdatetime.h>
#include <QDebug>
#include <qapplication.h>

#include <fstream>
#include <cmath>
#include <iostream>


// Constructeur de RtMidiHandler
RtMidiHandler::RtMidiHandler(QWidget* parent, std::shared_ptr<lomse::Interactor> interactor)
    : QObject(parent), midiIn(new RtMidiIn), isCapturing(false), m_interactor(interactor)
{
    // Initialiser RtMidiIn
    if (midiIn->getPortCount() == 0) {
        qDebug() << "Aucun port MIDI disponible.";
    }
    else {
        midiIn->openPort(0);  // Ouvre le premier port MIDI
        midiIn->setCallback(&RtMidiHandler::midiCallback, this);  // Lier callback MIDI
        midiIn->ignoreTypes(false, false, false);  // Ignorer les types de message

        std::ofstream logFile("log.txt", std::ios::trunc);
        logFile.close();
    }
}

// Destructeur de RtMidiHandler
RtMidiHandler::~RtMidiHandler()
{
    stopMidiCapture();
    delete midiIn;
}

// Demarre la capture MIDI
void RtMidiHandler::startMidiCapture()
{
    if (!isCapturing) {


        isCapturing = true;
        // On entre dans la portee
        lomse::CmdCursor* cmdEnter = new lomse::CmdCursor(lomse::CmdCursor::k_enter);
        m_interactor->exec_command(cmdEnter);
        
        // On passe la cle
        lomse::CmdCursor* cmdNext = new lomse::CmdCursor(lomse::CmdCursor::k_move_next);
        m_interactor->exec_command(cmdNext);
        
        // On passe la signature de temps
        lomse::CmdCursor* cmdNext2 = new lomse::CmdCursor(lomse::CmdCursor::k_move_next);
        m_interactor->exec_command(cmdNext2);
        
        // On passe le metronome
        lomse::CmdCursor* cmdNext3 = new lomse::CmdCursor(lomse::CmdCursor::k_move_next);
        m_interactor->exec_command(cmdNext3);

        qDebug() << "Demarrage de la capture MIDI...";

        if (auto* widget = qobject_cast<LomseWidget*>(parent())) {
            tempsParTemps = 60000.0 / widget->getTempo();
        }

        tempsDepart = QDateTime::currentMSecsSinceEpoch();
        
        logEvent("-- Debut de la partition --");
    }
}

// Arrête la capture MIDI
void RtMidiHandler::stopMidiCapture()
{
    if (isCapturing) {
        midiIn->cancelCallback();
        midiIn->closePort();  // Ferme le port MIDI
        isCapturing = false;
        qDebug() << "Capture MIDI arretee.";
        logEvent("-- Fin de la partition --");

        if (auto* widget = qobject_cast<LomseWidget*>(parent())) {
            widget->saveScoreAsPng("output.png");
        }
    }
}

// Ajoute une note au score
void RtMidiHandler::addNoteToScore(std::string type, std::string note, std::string duration)
{
    if (m_interactor != nullptr) {
        // Ajouter la note au score, à l'interactor
        // Mettre à jour l'affichage de la partition
        NoteCommand* noteCmd = new NoteCommand(type, note, duration, m_interactor);
        noteCmd->execute();

        if (auto* widget = qobject_cast<LomseWidget*>(parent())) {
            widget->refreshScore();
        }
        
        if (type == "n") {
            logEvent("Note: " + note + ", durée: " + duration);
        }
        else if (type == "r") {
            logEvent("Rest: durée: " + duration);
        }
        else if (note == "bar") {
            logEvent("--- Barre de mesure ---");
        }
    }
}

std::string RtMidiHandler::getNoteSymbol(unsigned char note)
{
    // Tableau des noms de notes en minuscules pour le format souhaité.
    const char* noteNames[] = { "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b" };

    // Calcul de l'octave selon la norme MIDI (C4 correspond à la note 60).
    int octave = (note / 12) - 1;

    // Construire la chaîne résultat, par exemple "c4" ou "g#5"
    std::stringstream ss;
    ss << noteNames[note % 12] << octave;
    return ss.str();
}



// Le callback pour capturer les messages MIDI
void RtMidiHandler::midiCallback(double deltatime, std::vector<unsigned char>* message, void* userData)
{
    RtMidiHandler* handler = static_cast<RtMidiHandler*>(userData);
    handler->processMidiMessage(*message);  // Traiter le message MIDI

    // Verifier le temps courant
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    if (now - handler->tempsDepart >= handler->tempsParTemps) {
        // nouveau depart du temps
        handler->tempsDepart = QDateTime::currentMSecsSinceEpoch();

        // ajout padding (rest) au temps
        // verifier si fin de mesure
        if (handler->tempsCourant + 1 == handler->nbTempsDansMesure) {
            handler->padMesure(false);
            // reset la mesure
            handler->tempsCourant = 0;
        }
        else {
            handler->padTemps();
            // incrementer le temps courant
            handler->tempsCourant++;
        }
    }
}

// Traite le message MIDI reçu
void RtMidiHandler::processMidiMessage(const std::vector<unsigned char>& message)
{
    // Verifier que le message est valide
    if (message.size() < 3) {
        return;
    }

    unsigned char status = message[0]; // Premier octet : statut du message MIDI
    unsigned char note = message[1];   // Deuxième octet : numero de note
    unsigned char velocity = message[2]; // Troisième octet : velocite

    double currentTime = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000.0;

    // Gestion du message "Note On" (status 0x90-0x9F)
    if (status >= 0x90 && status <= 0x9F) {
        // Message "Note On" (note enfoncee)
        if (velocity > 0) {
            //qDebug() << "Note pressee: " << static_cast<int>(note) << "Velocite: " << static_cast<int>(velocity);
            //addNoteToScore(note);  // Ajouter la note au score
            m_activeNotes[note] = currentTime;
        }
        else {
            if (m_activeNotes.count(note)) {
                double duration = currentTime - m_activeNotes[note];
                m_activeNotes.erase(note);
                handleNoteWithDuration(note, duration);
            }
        }
    }

    // Gestion du message "Note Off" (status 0x80-0x8F)
    else if (status >= 0x80 && status <= 0x8F) {
        // Message "Note Off" (note relachee)
        if (m_activeNotes.count(note)) {
            double duration = currentTime - m_activeNotes[note];
            m_activeNotes.erase(note);
            handleNoteWithDuration(note, duration);
        }
    }
}

void RtMidiHandler::handleNoteWithDuration(unsigned char note, double durationSec)
{
    std::string noteSymbol = getNoteSymbol(note);
    std::string duration;

    double beats = 0;

    if (durationSec < 0.25) {
        duration = "s"; // seizieme
        beats = 0.25;
    }
    else if (durationSec < 0.5) {
        duration = "e"; // croche
        beats = 0.5;
    }
    else if (durationSec < 1.0) {
        duration = "q"; // noire
        beats = 1.0;
    }
    else if (durationSec < 2.0) {
        duration = "h"; // blanche
        beats = 2.0;
    }
    else {
        duration = "w"; // ronde
        beats = 4;
    }

    // Ignore si on a déjà rempli la mesure
    if (contenuMesure + beats > nbTempsDansMesure)
        return;

    contenuMesure += beats;
    addNoteToScore("n", noteSymbol, duration);
}

void RtMidiHandler::padTemps() {
    if (m_activeNotes.empty()) {
        double partieEntiere;
        double partieFractionnaire = std::modf(contenuMesure, &partieEntiere);

        const double epsilon = 0.01;

        if (std::abs(partieFractionnaire - 0.0) < epsilon) {
            addNoteToScore("r", "r", "q");
            contenuMesure += 1.0;
        }
        else if (std::abs(partieFractionnaire - 0.25) < epsilon) {
            addNoteToScore("r", "r", "s");
            addNoteToScore("r", "r", "e");
            contenuMesure += 0.75;
        }
        else if (std::abs(partieFractionnaire - 0.5) < epsilon) {
            addNoteToScore("r", "r", "e");
            contenuMesure += 0.5;
        }
    }
}



void RtMidiHandler::padMesure(bool fin) {
    if (!m_activeNotes.empty()) {
        double tempsRestant = nbTempsDansMesure - contenuMesure;

        std::string noteSymbol = getNoteSymbol(m_activeNotes.begin()->first);
        m_activeNotes.clear();

        if (tempsRestant >= 2.0)
            addNoteToScore("n", noteSymbol, "h");
        else if (tempsRestant >= 1.0)
            addNoteToScore("n", noteSymbol, "q");
        else if (tempsRestant >= 0.5)
            addNoteToScore("n", noteSymbol, "e");
        else
            addNoteToScore("n", noteSymbol, "s");
    }
    else {
        padTemps();
    }
    addNoteToScore("bar", fin ? "end" : "bar", "");
    contenuMesure = 0;
}


void RtMidiHandler::logEvent(const std::string& message)
{
    std::ofstream logFile("log.txt", std::ios::app);
    if (logFile.is_open()) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        logFile << timestamp.toStdString() << " - " << message << std::endl;
        logFile.close();
    }
}

void RtMidiHandler::finalizeAndExit()
{
    if (m_interactor) {
        padMesure(true);
    }
    stopMidiCapture();   // Ferme proprement RtMidi
    logEvent("-- Fin de la partition (interruption manuelle) --");

    // Sauvegarde image ou PDF
    if (auto* widget = qobject_cast<LomseWidget*>(parent())) {
        if (widget) {
            widget->saveScoreAsPng("output.png");
        }
    }

    QTimer::singleShot(100, qApp, &QCoreApplication::quit);
}
