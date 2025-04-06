#include "RtMidiHandler.h"
#include "LomseViewWidget.h"
#include "NoteCommand.h"
#include <QDebug>
#include <iostream>
#include <lomse_presenter.h>
#include <lomse_interactor.h>
#include <lomse_doorway.h>
#include <lomse_command.h>
#include <lomse_interactor.h>

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

        lomse::CmdCursor* cmdEnter = new lomse::CmdCursor(lomse::CmdCursor::k_enter);
        m_interactor->exec_command(cmdEnter);

        qDebug() << "Demarrage de la capture MIDI...";
    }
}

// Arrête la capture MIDI
void RtMidiHandler::stopMidiCapture()
{
    if (isCapturing) {
        midiIn->closePort();  // Ferme le port MIDI
        isCapturing = false;
        qDebug() << "Capture MIDI arrêtee.";
    }
}

// Ajoute une note au score
void RtMidiHandler::addNoteToScore(unsigned char note)
{
    if (m_interactor != nullptr) {
        // Ajouter la note au score, à l'interactor
        // Mettre à jour l'affichage de la partition
        std::string noteSymbol = getNoteSymbol(note);
        NoteCommand* noteCmd = new NoteCommand(noteSymbol, m_interactor);
        noteCmd->execute();

        if (auto* widget = qobject_cast<LomseWidget*>(parent())) {
            widget->refreshScore();
        }
        
        qDebug() << "Note ajoutée au score : " << noteSymbol.c_str();
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

    // Gestion du message "Note On" (status 0x90-0x9F)
    if (status >= 0x90 && status <= 0x9F) {
        // Message "Note On" (note enfoncee)
        if (velocity > 0) {
            //qDebug() << "Note pressee: " << static_cast<int>(note) << "Velocite: " << static_cast<int>(velocity);
            addNoteToScore(note);  // Ajouter la note au score
        }
        else {
            //qDebug() << "Note relachee: " << static_cast<int>(note);
        }
    }

    // Gestion du message "Note Off" (status 0x80-0x8F)
    else if (status >= 0x80 && status <= 0x8F) {
        // Message "Note Off" (note relachee)
        qDebug() << "Note relachee: " << static_cast<int>(note);
    }
}
