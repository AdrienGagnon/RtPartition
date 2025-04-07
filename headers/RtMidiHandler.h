#pragma once

#include <RtMidi.h>
#include <QWidget>
#include <QTimer>
#include <vector>
#include <memory>
#include <string>
#include <lomse_document.h>
#include <lomse_interactor.h>

class RtMidiHandler : public QObject
{
    Q_OBJECT

public:
    RtMidiHandler(QWidget* parent = nullptr, std::shared_ptr<lomse::Interactor> interactor = nullptr);
    ~RtMidiHandler();

    void startMidiCapture();
    void stopMidiCapture();
    void addNoteToScore(std::string type, std::string note, std::string duration);
    void padTemps();
    void padMesure(bool fin);

    void finalizeAndExit();

private:

    std::unordered_map<unsigned char, double> m_activeNotes;
    std::shared_ptr<lomse::Interactor> m_interactor;

    qint64 tempsDepart = 0;
    qint64 tempsParTemps = 1000;
    double tempsCourant = 0.0;
    int nbTempsDansMesure = 4;
    double contenuMesure = 0.0;

    RtMidiIn* midiIn;
    bool isCapturing;

    static void midiCallback(double deltatime, std::vector<unsigned char>* message, void* userData);
    void processMidiMessage(const std::vector<unsigned char>& message);
    void handleNoteWithDuration(unsigned char note, double durationSec);
    std::string getNoteSymbol(unsigned char note);
    void logEvent(const std::string& message);
};
