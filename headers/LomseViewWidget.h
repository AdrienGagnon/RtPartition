#pragma once

#include "RtMidiHandler.h"
#include <QWidget>
#include <QImage>
#include <qlabel.h>
#include <memory>
#include <agg_rendering_buffer.h>

#include <lomse_doorway.h>

namespace lomse {
    class Interactor;
}

class LomseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LomseWidget(QWidget* parent = nullptr);
    ~LomseWidget();

    void saveScoreAsPng(const QString& filename);
    void refreshScore();
    int getTempo();

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    void initializeLomse();
    void loadScore();
    void renderScore();
    void startCountdownThenCapture();
    void keyPressEvent(QKeyEvent* event);

    int m_userTempo = 60;

    QLabel* m_countdownLabel;

    QImage m_scoreImage;
    lomse::LomseDoorway m_lomse;
    std::unique_ptr<lomse::Presenter> m_presenter;
    lomse::SpInteractor m_interactor;
    agg::rendering_buffer m_renderingBuffer;


    RtMidiHandler* midiHandler;
};
