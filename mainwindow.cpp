#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Samples Spectrum");
    setWindowIcon(QIcon{ QApplication::applicationDirPath() + "/ressources/icon.jpg" });

    if (QFontDatabase::addApplicationFont(":/FontAwesome.otf") < 0)
        qWarning() << "FontAwesome cannot be loaded !";

    m_centralWidget = new QWidget;
    m_centralLayout = new QVBoxLayout;
    m_tab = new QTabWidget;
    m_title = new QLabel;
    m_devicesList = new QListWidget;


    m_title->setFont(QFont("FontAwesome", 18));
    m_title->setText("<span>\uf83e</span> SamplesSpectrumVisualizer");

    m_centralLayout->addWidget(m_title);
    m_centralLayout->addWidget(m_devicesList);

    m_openAudioFile = new QPushButton("OPEN AUDIO FILE FROM DISK");

    /////////AUDIO INFORMATION//////////
    m_audioInformation = new QWidget;
    m_audioInformationLayout = new QVBoxLayout;

    m_duration = new QLabel("< unknow >");
    m_frequency = new QLabel("< unknow >");
    m_bitDepth = new QLabel("< unknow >");
    m_numSamples = new QLabel("< unknow >");
    m_numChannels = new QLabel("< unknow >");
    m_mono = new QLabel("< unknow >");
    m_stereo = new QLabel("< unknow >");

    m_audioInformationLayout->addWidget(m_duration);
    m_audioInformationLayout->addWidget(m_frequency);
    m_audioInformationLayout->addWidget(m_bitDepth);
    m_audioInformationLayout->addWidget(m_numSamples);
    m_audioInformationLayout->addWidget(m_numChannels);
    m_audioInformationLayout->addWidget(m_mono);
    m_audioInformationLayout->addWidget(m_stereo);

    m_audioInformation->setLayout(m_audioInformationLayout);

    m_tab->addTab(m_audioInformation, "Audio Information");

    /////////OPTIONS//////////
    m_options = new QWidget;
    m_optionsDrawingLayout = new QVBoxLayout;
    m_optionsDrawingBox = new QGroupBox("Drawing", m_options);
    m_drawModeDefault = new QRadioButton("Default");
    m_drawModeSinus = new QRadioButton("Sinus");
    m_drawModeCircle = new QRadioButton("Circle");
    m_drawModeFlower = new QRadioButton("Flower");

    m_drawModeDefault->setChecked(true);

    m_optionsDrawingLayout->addWidget(m_drawModeDefault);
    m_optionsDrawingLayout->addWidget(m_drawModeSinus);
    m_optionsDrawingLayout->addWidget(m_drawModeCircle);
    m_optionsDrawingLayout->addWidget(m_drawModeFlower);

    m_optionsDrawingBox->setLayout(m_optionsDrawingLayout);


    m_optionsMediaLayout = new QVBoxLayout;
    m_optionsMediaBox = new QGroupBox("Media Player", m_options);
    m_sliderValueLabel = new QLabel;
    m_audioElapsedSeconds = new QLabel;
    m_mediaStateLabel = new QLabel;

    m_mediaState = new ClickableLabel(m_mediaStateLabel);

    m_mediaState->setFont(QFont{"FontAwesome", 18});
    m_mediaState->setText("\uf04c");

    m_audioElapsedSeconds->setText("0 (s)");
    m_sliderValueLabel->setText("8000 (Hz)");
    m_frequencySlider = new QSlider(Qt::Orientation::Horizontal);
    m_frequencySlider->setMinimum(8000);
    m_frequencySlider->setMaximum(384000);


    m_optionsMediaLayout->addWidget(m_sliderValueLabel);
    m_optionsMediaLayout->addWidget(m_frequencySlider);
    m_optionsMediaLayout->addWidget(m_audioElapsedSeconds);
    m_optionsMediaLayout->addWidget(m_mediaState);

    m_optionsMediaBox->setLayout(m_optionsMediaLayout);

    m_optionsMainLayout = new QGridLayout;
    m_optionsMainLayout->addWidget(m_optionsDrawingBox);
    m_optionsMainLayout->addWidget(m_optionsMediaBox);


    m_options->setLayout(m_optionsMainLayout);
    m_tab->addTab(m_options, "Options");

    /////////APPLY//////////
    m_centralLayout->addWidget(m_tab);
    m_centralLayout->addWidget(m_openAudioFile);
    m_centralWidget->setLayout(m_centralLayout);

    /////////SIGNAU/SLOTS//////////
    QObject::connect(m_openAudioFile, &QPushButton::clicked, this, &MainWindow::openAudioFile);
    QObject::connect(m_drawModeDefault, static_cast<void (QRadioButton::*)(bool)>(&QRadioButton::toggled), this, [this](bool value) -> void {
        if (value)
            m_drawMode = DrawMode::DEFAULT;
        });

    QObject::connect(m_drawModeSinus, static_cast<void (QRadioButton::*)(bool)>(&QRadioButton::toggled), this, [this](bool value) -> void {
        if (value)
            m_drawMode = DrawMode::SINUS;
        });

    QObject::connect(m_drawModeCircle, static_cast<void (QRadioButton::*)(bool)>(&QRadioButton::toggled), this, [this](bool value) -> void {
        if (value)
            m_drawMode = DrawMode::CIRCLE;
        });
    QObject::connect(m_drawModeFlower, static_cast<void (QRadioButton::*)(bool)>(&QRadioButton::toggled), this, [this](bool value) -> void {
        if (value)
            m_drawMode = DrawMode::FLOWER;
        });

    QObject::connect(m_frequencySlider, static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged), this, [this](int value) -> void {
        m_spec.freq = value;
        m_sliderValueLabel->setText(QString::number(m_spec.freq) + " (Hz)");

        if(m_devID > 0)
        {
            auto previousDeviceID = m_devID;

            SDL_LockAudioDevice(m_devID);
            SDL_CloseAudioDevice(m_devID);
            m_devID = SDL_OpenAudioDevice(nullptr, 0, &m_spec, &m_spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
            SDL_PauseAudioDevice(m_devID, false);
            SDL_UnlockAudioDevice(previousDeviceID);
        }
    });

    QObject::connect(m_mediaState, &ClickableLabel::clicked, this, [this]() -> void
    {
        if(m_pauseState)
        {
            m_mediaState->setText("\uf04c");
            m_pauseState = false;
        }
        else
        {
            m_mediaState->setText("\uf04b");
            m_pauseState = true;
        }

        SDL_PauseAudioDevice(m_devID, m_pauseState);
    });

    QObject::connect(m_devicesList, &QListWidget::currentTextChanged, this, [this](const QString & currentText)->void{

        if(m_device == DeviceSelect::SPEAKER)
        {
            m_device = DeviceSelect::MICROPHONE;
            m_openAudioFile->setDisabled(true);
            m_optionsMediaBox->setDisabled(true);

            if(m_devID > 0)
            {
                m_spec.callback = [](void* param, Uint8* stream, int len) -> void
                {
                   reinterpret_cast<MainWindow*>(param)->callbackMicrophone(reinterpret_cast<float*>(stream), len / sizeof(float));
                };

                auto previousDeviceID = m_devID;
                SDL_LockAudioDevice(m_devID);
                SDL_CloseAudioDevice(m_devID);
                m_devID = SDL_OpenAudioDevice(nullptr, 1, &m_spec, &m_spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
                SDL_PauseAudioDevice(m_devID, false);
                SDL_UnlockAudioDevice(previousDeviceID);
            }
            else
            {
                m_spec.freq = 44100;
                m_spec.format = AUDIO_F32SYS;
                m_spec.channels = 1;
                m_spec.userdata = this;
                m_spec.samples = 4096;

                m_spec.callback = [](void* param, Uint8* stream, int len) -> void
                {
                   reinterpret_cast<MainWindow*>(param)->callbackMicrophone(reinterpret_cast<float*>(stream), len / sizeof(float));
                };

                m_devID = SDL_OpenAudioDevice(nullptr, 1, &m_spec, &m_spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
                SDL_PauseAudioDevice(m_devID, false);
            }


        }
        else
        {
            m_device = DeviceSelect::SPEAKER;
            m_openAudioFile->setDisabled(false);
            m_optionsMediaBox->setDisabled(false);
            SDL_CloseAudioDevice(m_devID);

        }

    });

    setCentralWidget(m_centralWidget);

    //////////CSS//////////
    setStyleSheet("background-color: #1A1A1A;color: #75C8D0;");
    m_openAudioFile->setStyleSheet("QPushButton:disabled{background-color: #2A2A2A;border: none;}");
    m_optionsMediaBox->setStyleSheet("QGroupBox:disabled {background-color: #2A2A2A;border: none;}");
    m_tab->setStyleSheet("QTabWidget::pane {border: none;color: red;}QTabBar::tab {\
                         background-color: #1A1A1A;\
                         border: 1px solid #333;\
                         padding: 10px\
                     } QTabBar::tab:hover{background-color: #2A2A2A;}");
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(100);
    effect->setColor(QColor("#75C8D0"));
    effect->setOffset(0, 0);

    m_title->setGraphicsEffect(effect);
    m_title->setStyleSheet("padding: 5px;");

}

MainWindow::~MainWindow()
{
    SDL_Quit();
}

void MainWindow::openAudioFile()
{
    m_audioFilePath = QFileDialog::getOpenFileName(this, "Open file", QString(), "Musics (*.wav)");

    SDL_CloseAudioDevice(m_devID);
    m_offset = 0;

    if (!m_audioFile.load(m_audioFilePath.toStdString()))
    {
        QMessageBox::critical(this, "Path Error", "Impossible to open :\n" + m_audioFilePath);
        return;
    }

    QMessageBox::information(this, "Loading", "Your file was loaded in memory !\n" + m_audioFilePath);

    const auto duration = m_audioFile.getLengthInSeconds();
    const auto frequency = m_audioFile.getSampleRate();
    const auto bitDepth = m_audioFile.getBitDepth();
    const auto numSamples = m_audioFile.getNumSamplesPerChannel();
    const auto numChannels = m_audioFile.getNumChannels();
    const auto isMono = m_audioFile.isMono();
    const auto isStereo = m_audioFile.isStereo();

    m_frequencySlider->setValue(static_cast<int>(frequency));
    m_frequencySlider->setMinimum(static_cast<int>(frequency) / 2);
    m_frequencySlider->setMaximum(static_cast<int>(frequency) * 2);

    m_duration->setText(QString::number(duration) + " (s)");
    m_frequency->setText(QString::number(frequency) + " (Hz)");
    m_bitDepth->setText(QString::number(bitDepth) + " (Bits)");
    m_numSamples->setText(QString::number(numSamples));
    m_numChannels->setText(QString::number(numChannels));

    if(isMono)
        m_mono->setText("true");
    else
        m_mono->setText("false");
    if (isStereo)
        m_stereo->setText("true");
    else
        m_stereo->setText("false");

    m_spec.freq = static_cast<int>(m_audioFile.getSampleRate());
    m_spec.format = AUDIO_F32SYS;
    m_spec.channels = 1;
    m_spec.userdata = this;
    m_spec.samples = 800;

    m_spec.callback = [](void* param, Uint8* stream, int len) -> void
    {
        reinterpret_cast<MainWindow*>(param)->callbackAudioFile(reinterpret_cast<float*>(stream), len / sizeof(float));
    };

    if(m_devID > 0)
    {
        auto previousDeviceID = m_devID;
        SDL_LockAudioDevice(m_devID);
        SDL_CloseAudioDevice(m_devID);
        m_devID = SDL_OpenAudioDevice(nullptr, 0, &m_spec, &m_spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
        SDL_PauseAudioDevice(m_devID, false);
        SDL_UnlockAudioDevice(previousDeviceID);
    }
    else
    {
        m_devID = SDL_OpenAudioDevice(nullptr, 0, &m_spec, &m_spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
        SDL_PauseAudioDevice(m_devID, false);
    }
}

void MainWindow::callbackAudioFile(float* samples, const int samplesSize)
{
    static int countSamples = 0;
    int referentialSampleRate = m_audioFile.getSampleRate();
    double coef = referentialSampleRate / static_cast<double>(m_spec.freq);


    if ((m_audioFile.getNumSamplesPerChannel() / samplesSize) == m_offset)
        m_offset = 0;

    for (auto i = 0; i < samplesSize; i++)
    {
        samples[i] = m_audioFile.samples[0][i + m_offset * samplesSize];
    }

    countSamples++;

    if(countSamples >= (m_spec.freq / m_spec.samples) * coef)
    {
        m_secondsElapsed ++;
        m_audioElapsedSeconds->setText(QString::number(m_secondsElapsed) + " (s)");
        countSamples = 0;
    }
    m_offset++;
    draw(samples, samplesSize);
}

void MainWindow::callbackMicrophone(float* samples, const int samplesSize)
{
    for (auto i = 0; i < samplesSize; i++)
    {
        samples[i] *= 4;
    }
    draw(samples, samplesSize);
}

bool MainWindow::initializeSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        return false;

    m_window = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(SDL_CreateWindow("SamplesViewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, SDL_WINDOW_SHOWN), &SDL_DestroyWindow);

    if (m_window == nullptr)
        return false;

    m_renderer = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>(SDL_CreateRenderer(m_window.get(), -1, SDL_RENDERER_ACCELERATED), SDL_DestroyRenderer);

    if (m_renderer == nullptr)
        return false;


    for(auto i = 0; i < SDL_GetNumAudioDevices(0); i++)
    {
        QString deviceName = SDL_GetAudioDeviceName(i, 0);
        m_devicesList->addItem("\uf025 " + deviceName);
    }

    for(auto i = 0; i < SDL_GetNumAudioDevices(0); i++)
    {
        QString deviceName = SDL_GetAudioDeviceName(i, 1);
        m_devicesList->addItem("\uf130 "+ deviceName);
    }

    SDL_SetWindowBordered(m_window.get(), SDL_FALSE);
    SDL_SetRenderDrawColor(m_renderer.get(), 0xC0, 0xC0, 0xC0, 255);
    SDL_RenderClear(m_renderer.get());

    SDL_SetRenderDrawColor(m_renderer.get(), 255, 255, 255, 255);
    SDL_RenderDrawLine(m_renderer.get(), 0, 600 / 2, 800, 600 / 2);

    return true;
}

void MainWindow::draw(float* samples, const int length)
{
    SDL_SetRenderDrawColor(m_renderer.get(), 0xC0, 0xC0, 0xC0, 255);
    SDL_RenderClear(m_renderer.get());

    SDL_SetRenderDrawColor(m_renderer.get(), 255, 255, 255, 255);
    SDL_RenderDrawLine(m_renderer.get(), 0, 600 / 2, 800, 600 / 2);

    if (m_drawMode == DrawMode::DEFAULT)
    {

        for (auto i = 0; i < length; i++)
        {
            SDL_SetRenderDrawColor(m_renderer.get(), 0x34, 0x30, 0xCB, 255);
            SDL_RenderDrawLine(m_renderer.get(), i * m_zoom, 300 * samples[i] + (600 / 2), i * m_zoom, 600 / 2);

            SDL_SetRenderDrawColor(m_renderer.get(), 0x64, 0x64, 0xDC, 255);
            SDL_RenderDrawLine(m_renderer.get(), i * m_zoom, (150 * samples[i] + (600 / 2)), i * m_zoom, 600 / 2);
        }

        SDL_RenderPresent(m_renderer.get());
    }

    if (m_drawMode == DrawMode::SINUS)
    {
        std::vector<SDL_Point> points;

        for (auto i = 0; i < length; i++)
        {
            points.push_back({ static_cast<int>(i * m_zoom), static_cast<int>(300 * samples[i] + SDL_WINDOW_HEIGHT / 2.0f) });
        }

        SDL_SetRenderDrawColor(m_renderer.get(), 0x34, 0x30, 0xCB, 255);
        SDL_RenderDrawLines(m_renderer.get(), &points[0], static_cast<int>(points.size()));
        SDL_RenderPresent(m_renderer.get());
    }

    if (m_drawMode == DrawMode::FLOWER)
    {

        SDL_SetRenderDrawColor(m_renderer.get(), 0x34, 0x30, 0xCB, 255);

        std::vector<SDL_Point> points;

        for (auto i = 0; i < length; i++)
        {
            points.push_back({ static_cast<int>( SDL_WINDOW_WIDTH / 2 + 300 * samples[i] * qCos(i * M_PI / 180.0f) ), static_cast<int>( SDL_WINDOW_HEIGHT / 2 + 300 * samples[i] * qSin(i * M_PI / 180.0f) )});
            //SDL_RenderDrawLine(m_renderer.get(), m_w / 2, m_h / 2, m_w / 2 + 300 * samples[i] * qCos(i * M_PI / 180.0f), m_h / 2 + 300 * samples[i] * qSin(i * M_PI / 180.0f));
        }


        SDL_RenderDrawLines(m_renderer.get(), &points[0], static_cast<int>(points.size()));

        SDL_RenderPresent(m_renderer.get());
    }

    if (m_drawMode == DrawMode::CIRCLE)
    {

        SDL_SetRenderDrawColor(m_renderer.get(), 0x34, 0x30, 0xCB, 255);

        for (auto i = 0; i < length; i++)
        {
            SDL_RenderDrawLine(m_renderer.get(), SDL_WINDOW_WIDTH / 2, SDL_WINDOW_HEIGHT / 2, SDL_WINDOW_WIDTH / 2 + 300 * samples[i] * qCos(i * M_PI / 180.0f), SDL_WINDOW_HEIGHT / 2 + 300 * samples[i] * qSin(i * M_PI / 180.0f));
        }

        SDL_RenderPresent(m_renderer.get());
    }
}
