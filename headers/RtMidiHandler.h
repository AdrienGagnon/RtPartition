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
    void addNoteToScore(unsigned char note);

private:
    static void midiCallback(double deltatime, std::vector<unsigned char>* message, void* userData);
    void processMidiMessage(const std::vector<unsigned char>& message);

    RtMidiIn* midiIn;
    bool isCapturing;
    std::shared_ptr<lomse::Interactor> m_interactor;
    std::string getNoteSymbol(unsigned char note);
};
