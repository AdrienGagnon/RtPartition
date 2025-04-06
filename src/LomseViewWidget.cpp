#include "LomseViewWidget.h"
#include "RtMidiHandler.h"
#include <lomse_doorway.h>
#include <lomse_presenter.h>
#include <lomse_interactor.h>
#include <lomse_document.h>
#include <qcoreapplication.h>

#include <iostream>
#include <QPainter>
#include <QDebug>

using namespace lomse;

LomseWidget::LomseWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(800, 600); // Force une taille initiale suffisante
    initializeLomse();
    loadScore();
    renderScore();


    midiHandler = new RtMidiHandler(this, m_interactor);  // Assurez-vous que RtMidiHandler est une classe qui gere MIDI
    midiHandler->startMidiCapture();  // Demarrer la capture MIDI
}

LomseWidget::~LomseWidget() {
    delete midiHandler;  // Liberer RtMidiHandler
}

void LomseWidget::initializeLomse()
{
    m_lomse.init_library(k_pix_format_rgba32, 96, false, std::cerr);
    m_lomse.set_default_fonts_path("C:/lomse/fonts");
}

void LomseWidget::loadScore()
{
    qDebug() << "Load";
    std::string ldp =
        "(score (vers 2.0)\n"
        "  (instrument (name \"Piano\") (staves 1)\n"
        "    (musicData\n"
        "      (clef G)\n"
        "      (key C)\n"
        "      (time 4 4)\n"
        "      (n c4 q)\n"
        "      (n e4 q)\n"
        "      (n g4 e)\n"
        "      (n f4 e)\n"
        "      (barline simple)\n"
        "      (n d4 q)\n"
        "      (chord (n c4 e) (n e4 e))\n"
        "      (n g4 q)\n"
        "    )\n"
        "  )\n"
        ")";

    //qDebug() << "Score LDP: " << QString::fromStdString(ldp);

    #define k_view_simple 0

    m_presenter.reset(m_lomse.new_document(
        Document::k_format_ldp,
        ldp,
        k_view_simple,
        std::cout
    ));

    m_interactor = m_presenter->get_interactor(0).lock();
}

void LomseWidget::renderScore()
{
    qDebug() << "Render";

    if (!m_interactor) {
        qDebug() << "Erreur: interactor non disponible";
        return;
    }

    int w = width();
    int h = height();

    m_scoreImage = QImage(w, h, QImage::Format_ARGB32);
    m_scoreImage.fill(Qt::white);

    m_renderingBuffer.attach(
        m_scoreImage.bits(),
        m_scoreImage.width(),
        m_scoreImage.height(),
        m_scoreImage.bytesPerLine()
    );

    m_interactor->set_rendering_buffer(&m_renderingBuffer);
    m_interactor->redraw_bitmap();

    // Sauvegarde pour debug
    //m_scoreImage.save("output.png");
    //qDebug() << "Image sauvegardee dans output.png";

    update();
}

void LomseWidget::paintEvent(QPaintEvent*)
{
    qDebug() << "Peinture de l'evenement";
    QPainter painter(this);
    painter.drawImage(0, 0, m_scoreImage);
}

void LomseWidget::resizeEvent(QResizeEvent*)
{
    renderScore();
}

void LomseWidget::refreshScore()
{
    if (!m_interactor) {
        qDebug() << "Erreur: interactor non disponible pour refresh.";
        return;
    }

    m_interactor->redraw_bitmap(); 
    update();
}
