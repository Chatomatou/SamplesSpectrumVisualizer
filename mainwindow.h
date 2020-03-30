#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QFontDatabase>
#include <QMainWindow>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QSlider>
#include <QtMath>
#include <QListWidget>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include "ClickableLabel.h"


#include <SDL.h>
#include <memory>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>

#include "AudioFile.h"

#include "DrawMode.h"
#include "DeviceSelect.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();

    bool initializeSDL();
    void draw(float* samples, const int length);
    void callbackAudioFile(float* samples, const int samplesSize);
    void callbackMicrophone(float* samples, const int samplesSize);
public slots:
    void openAudioFile();
private:
    // Widgets
    QWidget* m_centralWidget;
    QVBoxLayout* m_centralLayout;

    // General
    QTabWidget* m_tab;
    QPushButton* m_openAudioFile;
    QLabel* m_title;
    QListWidget* m_devicesList;

    // Information
    QWidget* m_audioInformation;
    QVBoxLayout* m_audioInformationLayout;
    QLabel* m_duration;
    QLabel* m_frequency;
    QLabel* m_bitDepth;
    QLabel* m_numSamples;
    QLabel* m_numChannels;
    QLabel* m_mono;
    QLabel* m_stereo;

    // Options
    QWidget* m_options;
    QGridLayout* m_optionsMainLayout;

    QVBoxLayout* m_optionsDrawingLayout;
    QGroupBox* m_optionsDrawingBox;
    QRadioButton* m_drawModeDefault;
    QRadioButton* m_drawModeSinus;
    QRadioButton* m_drawModeCircle;
    QRadioButton* m_drawModeFlower;

    QVBoxLayout* m_optionsMediaLayout;
    QGroupBox* m_optionsMediaBox;
    QSlider* m_frequencySlider;
    QLabel* m_sliderValueLabel;
    QLabel* m_audioElapsedSeconds;
    QLabel* m_mediaStateLabel;
    ClickableLabel* m_mediaState;

    QHBoxLayout* m_align;

    // SDL
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> m_window{ nullptr, nullptr };
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> m_renderer{ nullptr, nullptr };
    SDL_AudioSpec m_spec;
    SDL_AudioDeviceID m_devID{0};

    // Own
    AudioFile<double> m_audioFile;
    QString m_audioFilePath;
    int m_zoom{1};
    DrawMode m_drawMode{DrawMode::DEFAULT};
    DeviceSelect m_device{DeviceSelect::MICROPHONE};
    int m_offset{ 0 };
    int m_secondsElapsed{ 0 };
    bool m_pauseState{false};
    static constexpr unsigned int SDL_WINDOW_WIDTH{800};
    static constexpr unsigned int SDL_WINDOW_HEIGHT{600};
};
#endif // MAINWINDOW_H
