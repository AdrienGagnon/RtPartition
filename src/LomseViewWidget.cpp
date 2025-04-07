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
#include <qlabel.h>
#include <qfile.h>
#include <qdir.h>
#include <qfontdatabase.h>
#include <qinputdialog.h>
#include <QKeyEvent>
#include <qpointer.h>

using namespace lomse;

LomseWidget::LomseWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(800, 600);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    
    initializeLomse();
    loadScore();
    renderScore();
    
    startCountdownThenCapture();
}

LomseWidget::~LomseWidget() {
    delete midiHandler;
}

void LomseWidget::initializeLomse()
{
    m_lomse.init_library(k_pix_format_rgba32, 96, true, std::cerr);

    LibraryScope* pScope = m_lomse.get_library_scope();
    pScope->set_music_font("Bravura.otf", "Bravura", "C:/lomse/fonts");

    // Decommenter pour avoir le input du tempo par l'utilisateur
    bool ok = false;
    int tempo = QInputDialog::getInt(this, "Choisir le tempo", "Noire = ", 100, 30, 240, 1, &ok);
    if (ok)
        m_userTempo = tempo;

    // Decompte
    m_countdownLabel = new QLabel(this);
    m_countdownLabel->setAlignment(Qt::AlignCenter);
    m_countdownLabel->setStyleSheet("font-size: 48px; color: red;");
    m_countdownLabel->setGeometry(rect());  // Prend toute la taille du widget
    m_countdownLabel->hide();               // Masqué au départ


    m_userTempo = 60;
}

void LomseWidget::loadScore()
{
    std::ostringstream oss;
    oss <<
        "(score (vers 2.0)\n"
        "  (instrument (name \"Piano\") (staves 1)\n"
        "    (musicData\n"
        "      (clef G)\n"
        "      (time 4 4)\n"
        "      (metronome q " << m_userTempo << ")\n"
        "    )\n"
        "  )\n"
        ")";
    std::string ldp = oss.str();

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

    update();
}

void LomseWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(0, 0, m_scoreImage);
}

void LomseWidget::resizeEvent(QResizeEvent*)
{
    if (m_countdownLabel)
        m_countdownLabel->setGeometry(rect());
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

void LomseWidget::startCountdownThenCapture()
{
    QStringList messages = { "3", "2", "1", "Go!" };
    int delay = 1000;

    QPointer<LomseWidget> safeThis(this);
    m_countdownLabel->show();

    for (int i = 0; i < messages.size(); ++i) {
        QTimer::singleShot(i * delay, this, [safeThis, msg = messages[i]]() {
            if (!safeThis) return;
            safeThis->m_countdownLabel->setText(msg);
            });
    }

    QTimer::singleShot(messages.size() * delay, this, [safeThis]() {
        if (!safeThis) return;
        safeThis->m_countdownLabel->hide();

        safeThis->midiHandler = new RtMidiHandler(safeThis, safeThis->m_interactor);
        safeThis->midiHandler->startMidiCapture();
        });
}


int LomseWidget::getTempo() {
    return m_userTempo;
}

void LomseWidget::saveScoreAsPng(const QString& filename)
{
    m_scoreImage.save(filename);
    qDebug() << "Partition sauvegardee dans : " << filename;
}

void LomseWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_E) {
        qDebug() << "Sortie de l'application...";

        if (midiHandler) {
            midiHandler->finalizeAndExit();  // méthode à définir dans RtMidiHandler
        }
    }
}
