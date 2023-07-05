//
// Created by Conan Maël on 03/07/2023.
//
#pragma once

#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QVBoxLayout>
//... Include your image processing functions here
#include "image.hh"
#include "image_operations.hh"
#include "image_to_qt.hh"

class SquareButton : public QPushButton
{
    Q_OBJECT

public:
    SquareButton(const QString& text, QWidget* parent = nullptr)
        : QPushButton(text, parent)
    {}

    QSize sizeHint() const override
    {
        QSize size = QPushButton::sizeHint();
        int length = qMax(size.width(), size.height());
        return QSize(length, length);
    }

    QSize minimumSizeHint() const override
    {
        QSize size = QPushButton::minimumSizeHint();
        int length = qMax(size.width(), size.height());
        return QSize(length, length);
    }
};

class ResizableScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit ResizableScrollArea(QWidget* parent = nullptr,
                                 Qt::WindowFlags f = Qt::WindowFlags())
        : QScrollArea(parent)
    {}

signals:
    void resized();

protected:
    void resizeEvent(QResizeEvent* event) override
    {
        QScrollArea::resizeEvent(event);
        emit resized();
    }
};

class ResizableImageLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ResizableImageLabel(QWidget* parent = nullptr,
                                 Qt::WindowFlags f = Qt::WindowFlags())
        : QLabel(parent, f)
    {
        if (parent)
            connect(parent, SIGNAL(resized()), this, SLOT(updatePixmap()));
    }

public slots:
    void updatePixmap()
    {
        auto parentWidget = qobject_cast<QWidget*>(this->parent());
        if (parentWidget)
        {
            auto parentSize = parentWidget->size();
            QLabel::setPixmap(originalPixmap.scaled(
                parentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    void setPixmap(const QPixmap& p)
    {
        originalPixmap = p;
        updatePixmap();
    }

private:
    QPixmap originalPixmap;
};

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow()
    {
        index = 0;
        // Main layout
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // Top buttons
        QHBoxLayout* topButtonsLayout = new QHBoxLayout();
        SquareButton* loadButton = new SquareButton("Load ", this);
        connect(loadButton, &QPushButton::clicked, this,
                &MainWindow::loadImage);
        topButtonsLayout->addWidget(loadButton);

        SquareButton* saveButton = new SquareButton("Save ", this);
        connect(saveButton, &QPushButton::clicked, this,
                &MainWindow::saveImage);
        saveButton->setEnabled(false);
        this->saveButton = saveButton;
        topButtonsLayout->addWidget(saveButton);

        SquareButton* backwardButton = new SquareButton("   ↩   ", this);
        connect(backwardButton, &QPushButton::clicked, this,
                &MainWindow::backwardImage);
        backwardButton->setEnabled(false);
        this->backwardButton = backwardButton;
        topButtonsLayout->addWidget(backwardButton);

        SquareButton* forwardButton = new SquareButton("   ↪   ", this);
        connect(forwardButton, &QPushButton::clicked, this,
                &MainWindow::forwardImage);
        forwardButton->setEnabled(false);
        this->forwardButton = forwardButton;
        topButtonsLayout->addWidget(forwardButton);

        SquareButton* originalButton = new SquareButton("Reset", this);
        connect(originalButton, &QPushButton::clicked, this,
                &MainWindow::originalImage);
        originalButton->setEnabled(false);
        this->originalButton = originalButton;
        topButtonsLayout->addWidget(originalButton);

        mainLayout->addLayout(topButtonsLayout);

        // Image and options
        QHBoxLayout* imageAndOptionsLayout = new QHBoxLayout();
        mainLayout->addLayout(imageAndOptionsLayout);

        // Image area
        ResizableScrollArea* scrollArea = new ResizableScrollArea;
        m_imageLabel = new ResizableImageLabel(scrollArea);
        scrollArea->setWidgetResizable(true);
        scrollArea->setWidget(m_imageLabel);
        imageAndOptionsLayout->addWidget(scrollArea);

        // Options area
        QVBoxLayout* optionsLayout = new QVBoxLayout;

        // Create the scroll area and set its widget to a new QWidget that
        // contains the options layout
        QScrollArea* optionsScrollArea = new QScrollArea;
        optionsScrollArea->setMaximumWidth(400);
        QWidget* optionsWidget = new QWidget;
        optionsWidget->setEnabled(false);
        this->optionsWidget = optionsWidget;
        optionsWidget->setLayout(optionsLayout);
        optionsScrollArea->setWidget(optionsWidget);
        optionsScrollArea->setWidgetResizable(true);
        imageAndOptionsLayout->addWidget(optionsScrollArea);

        /**
         ** FILTERS PART
         **/

        QPushButton* toggleFiltersButton = new QPushButton("> Filters");
        optionsLayout->addWidget(toggleFiltersButton);

        QGroupBox* FiltersGroup = new QGroupBox();
        FiltersGroup->setMaximumHeight(800);
        FiltersGroup->setCheckable(false); // Not checkable
        FiltersGroup->setVisible(false); // Initially not visible
        optionsLayout->addWidget(FiltersGroup);

        QVBoxLayout* filtersCheckBoxLayout = new QVBoxLayout;
        FiltersGroup->setLayout(filtersCheckBoxLayout);

        QPushButton* filmFilterButton = new QPushButton("Film Filter", this);
        connect(filmFilterButton, &QPushButton::clicked, this, [this]() {
            applyFilter(tifo::argentique_filter, "Argentique");
        });
        filtersCheckBoxLayout->addWidget(filmFilterButton);

        QPushButton* IRfilterButton = new QPushButton("IR Filter", this);
        connect(IRfilterButton, &QPushButton::clicked, this,
                [this]() { applyFilter(tifo::ir_filter, "IR"); });
        filtersCheckBoxLayout->addWidget(IRfilterButton);

        QPushButton* negativeFilterButton =
            new QPushButton("Negative Filter", this);
        connect(negativeFilterButton, &QPushButton::clicked, this,
                [this]() { applyFilter(tifo::negative_filter, "Negative"); });
        filtersCheckBoxLayout->addWidget(negativeFilterButton);

        QPushButton* grayscaleFilterButton =
            new QPushButton("Grayscale Filter", this);
        connect(grayscaleFilterButton, &QPushButton::clicked, this,
                [this]() { applyFilter(tifo::grayscale, "Grayscale"); });
        filtersCheckBoxLayout->addWidget(grayscaleFilterButton);

        // GLOW FILTER

        QVBoxLayout* glowFilterLayout = new QVBoxLayout;
        QVBoxLayout* glowRadiusLayout = new QVBoxLayout;
        QVBoxLayout* glowThresholdLayout = new QVBoxLayout;

        QHBoxLayout* glowRadiusSliderLayout = new QHBoxLayout;
        QHBoxLayout* glowThresholdSliderLayout = new QHBoxLayout;

        QLabel* minGlowRadius = new QLabel("0");
        QLabel* maxGlowRadius = new QLabel("400");

        QLabel* minGlowThreshold = new QLabel("0");
        QLabel* maxGlowThreshold = new QLabel("255");

        QLabel* valueGlowRadius = new QLabel("Glow Radius: 0");
        QLabel* valueGlowThreshold = new QLabel("Glow Threshold: 0");

        QSlider* glowRadiusSlider = new QSlider(Qt::Horizontal);
        QSlider* glowThresholdSlider = new QSlider(Qt::Horizontal);

        glowRadiusSlider->setRange(0, 400);
        glowRadiusSlider->setValue(0);
        glowRadiusSlider->setMaximumWidth(300);

        glowThresholdSlider->setRange(0, 255);
        glowThresholdSlider->setValue(0);
        glowThresholdSlider->setMaximumWidth(300);

        glowRadiusSliderLayout->addWidget(minGlowRadius);
        glowRadiusSliderLayout->addWidget(glowRadiusSlider);
        glowRadiusSliderLayout->addWidget(maxGlowRadius);

        glowThresholdSliderLayout->addWidget(minGlowThreshold);
        glowThresholdSliderLayout->addWidget(glowThresholdSlider);
        glowThresholdSliderLayout->addWidget(maxGlowThreshold);

        glowRadiusLayout->addLayout(glowRadiusSliderLayout);
        glowRadiusLayout->addWidget(valueGlowRadius);

        glowThresholdLayout->addLayout(glowThresholdSliderLayout);
        glowThresholdLayout->addWidget(valueGlowThreshold);

        glowFilterLayout->addLayout(glowRadiusLayout);
        glowFilterLayout->addLayout(glowThresholdLayout);

        QPushButton* glowFilterButton =
            new QPushButton("Apply glow filter", this);
        connect(glowFilterButton, &QPushButton::clicked, this,
                [this]() { applyGlow(tifo::glow_filter, 200.0, 127, "Glow"); });
        glowFilterLayout->addWidget(glowFilterButton);

        filtersCheckBoxLayout->addLayout(glowFilterLayout);

        //

        QPushButton* horizontalFilterButton =
            new QPushButton("Horizontal Flip", this);
        connect(horizontalFilterButton, &QPushButton::clicked, this, [this]() {
            applyFilter(tifo::horizontal_flip, "Horizontal Flip");
        });
        filtersCheckBoxLayout->addWidget(horizontalFilterButton);

        QPushButton* verticalFilterButton =
            new QPushButton("Vertical Flip", this);
        connect(verticalFilterButton, &QPushButton::clicked, this, [this]() {
            applyFilter(tifo::vertical_flip, "Vertical Flip");
        });
        filtersCheckBoxLayout->addWidget(verticalFilterButton);

        connect(toggleFiltersButton, &QPushButton::clicked, [=]() {
            bool isVisible = FiltersGroup->isVisible();
            FiltersGroup->setVisible(!isVisible);
            if (isVisible)
            {
                toggleFiltersButton->setText("> Filters");
            }
            else
            {
                toggleFiltersButton->setText("V Filters");
            }
        });

        /**
         ** SWAP CHANNELS PART
         **/

        QPushButton* toggleChangeChannelsButton =
            new QPushButton("> Channels Swap");
        optionsLayout->addWidget(toggleChangeChannelsButton);

        QGroupBox* SwapChannelsGroup = new QGroupBox();
        SwapChannelsGroup->setMaximumHeight(100);
        SwapChannelsGroup->setCheckable(false); // Not checkable
        SwapChannelsGroup->setVisible(false); // Initially not visible
        optionsLayout->addWidget(SwapChannelsGroup);

        QVBoxLayout* SwapChannelLayout = new QVBoxLayout;
        SwapChannelsGroup->setLayout(SwapChannelLayout);

        QHBoxLayout* SwapChannelsCheckBoxLayout = new QHBoxLayout;
        SwapChannelLayout->addLayout(SwapChannelsCheckBoxLayout);

        SwapChannelsCheckBoxLayout->addStretch(1);

        QCheckBox* redCheckBox = new QCheckBox("Red", this);
        SwapChannelsCheckBoxLayout->addWidget(redCheckBox);

        SwapChannelsCheckBoxLayout->addStretch(1);

        QCheckBox* greenCheckBox = new QCheckBox("Green", this);
        SwapChannelsCheckBoxLayout->addWidget(greenCheckBox);

        SwapChannelsCheckBoxLayout->addStretch(1);

        QCheckBox* blueCheckBox = new QCheckBox("Blue", this);
        SwapChannelsCheckBoxLayout->addWidget(blueCheckBox);

        SwapChannelsCheckBoxLayout->addStretch(1);

        QPushButton* SwapChannelValidate = new QPushButton("Apply", this);
        SwapChannelLayout->addWidget(SwapChannelValidate);
        SwapChannelValidate->setEnabled(false);

        auto lambdaFunc = [redCheckBox, blueCheckBox, greenCheckBox,
                           SwapChannelValidate]() {
            int totalChecked = redCheckBox->isChecked()
                + blueCheckBox->isChecked() + greenCheckBox->isChecked();
            SwapChannelValidate->setEnabled(totalChecked == 2);
        };

        connect(greenCheckBox, &QCheckBox::clicked, this, lambdaFunc);
        connect(redCheckBox, &QCheckBox::clicked, this, lambdaFunc);
        connect(blueCheckBox, &QCheckBox::clicked, this, lambdaFunc);

        connect(SwapChannelValidate, &QPushButton::clicked, this,
                [this, redCheckBox, greenCheckBox]() {
                    int channel1, channel2;

                    if (redCheckBox->isChecked())
                    {
                        channel1 = RED;
                        if (greenCheckBox->isChecked())
                            channel2 = GREEN;
                        else
                            channel2 = BLUE;
                    }
                    else
                    {
                        channel1 = GREEN;
                        channel2 = BLUE;
                    }
                    MainWindow::applySwap(tifo::swap_channels, channel1,
                                          channel2, "Swap");
                });

        connect(toggleChangeChannelsButton, &QPushButton::clicked, [=]() {
            bool isVisible = SwapChannelsGroup->isVisible();
            SwapChannelsGroup->setVisible(!isVisible);
            if (isVisible)
            {
                toggleChangeChannelsButton->setText("> Channels Swap");
            }
            else
            {
                toggleChangeChannelsButton->setText("V Channels Swap");
            }
        });

        /**
         ** CHANNELS INCREASING
         **/

        QPushButton* toggleChannelsButton =
            new QPushButton("> Channel increasing");
        optionsLayout->addWidget(toggleChannelsButton);

        QGroupBox* channelsGroup = new QGroupBox();
        channelsGroup->setMaximumHeight(200);
        channelsGroup->setCheckable(false); // Not checkable
        channelsGroup->setVisible(false); // Initially not visible
        optionsLayout->addWidget(channelsGroup);

        QVBoxLayout* checkBoxChannelLayout = new QVBoxLayout;
        channelsGroup->setLayout(checkBoxChannelLayout);

        // SLIDER 1: SATURATION
        QHBoxLayout* redLayout = new QHBoxLayout;
        QSlider* redSlider = new QSlider(Qt::Horizontal);
        redSlider->setRange(-255, 255);
        redSlider->setValue(0);
        redSlider->setMaximumWidth(300); // Set to appropriate value
        QLabel* redMinLabel = new QLabel("-255");
        QLabel* redMaxLabel = new QLabel("255");
        QLabel* redValueLabel = new QLabel;
        redValueLabel->setText(
            QString("Red increase: %1").arg(redSlider->value()));
        redLayout->addWidget(redMinLabel);

        redLayout->addWidget(redSlider);
        redLayout->addWidget(redMaxLabel);

        QVBoxLayout* redVLayout = new QVBoxLayout;
        redVLayout->addLayout(redLayout);

        QHBoxLayout* red_Layout = new QHBoxLayout;
        red_Layout->addStretch(1); // add a stretchable empty space at left
        red_Layout->addWidget(redValueLabel); // add your label
        red_Layout->addStretch(1); // add a stretchable empty space at right

        redVLayout->addLayout(red_Layout);

        checkBoxChannelLayout->addLayout(redVLayout);

        red_slider_value = 0;

        connect(redSlider, &QSlider::sliderPressed, [=, this]() {
            qDebug() << "Initial value: " << redSlider->value();
            red_slider_value = redSlider->value();
        });

        connect(redSlider, &QSlider::sliderReleased, [=, this]() {
            qDebug() << "Final value: " << redSlider->value();
            auto old_value = red_slider_value;
            auto new_value = redSlider->value();
            applySwap(tifo::increase_channel, new_value - old_value, RED,
                      "Red increase");
            red_slider_value = new_value;
        });

        connect(redSlider, &QSlider::valueChanged, [=, this](int value) {
            redValueLabel->setText(
                QString("Red increase: %1").arg(redSlider->value()));
        });

        // SLIDER 2: GREEN
        QHBoxLayout* greenLayout = new QHBoxLayout;
        QSlider* greenSlider = new QSlider(Qt::Horizontal);
        greenSlider->setRange(-255, 255);
        greenSlider->setValue(0);
        greenSlider->setMaximumWidth(300); // Set to appropriate value
        QLabel* greenMinLabel = new QLabel("-255");
        QLabel* greenMaxLabel = new QLabel("255");
        QLabel* greenValueLabel = new QLabel;
        greenValueLabel->setText(
            QString("Green increase: %1").arg(greenSlider->value()));
        greenLayout->addWidget(greenMinLabel);

        greenLayout->addWidget(greenSlider);
        greenLayout->addWidget(greenMaxLabel);

        QVBoxLayout* greenVLayout = new QVBoxLayout;
        greenVLayout->addLayout(greenLayout);

        QHBoxLayout* green_Layout = new QHBoxLayout;
        green_Layout->addStretch(1); // add a stretchable empty space at left
        green_Layout->addWidget(greenValueLabel); // add your label
        green_Layout->addStretch(1); // add a stretchable empty space at right

        greenVLayout->addLayout(green_Layout);

        checkBoxChannelLayout->addLayout(greenVLayout);

        green_slider_value = 0;

        connect(greenSlider, &QSlider::sliderPressed, [=, this]() {
            qDebug() << "Initial value: " << greenSlider->value();
            green_slider_value = greenSlider->value();
        });

        connect(greenSlider, &QSlider::sliderReleased, [=, this]() {
            qDebug() << "Final value: " << greenSlider->value();
            auto old_value = green_slider_value;
            auto new_value = greenSlider->value();
            applySwap(tifo::increase_channel, new_value - old_value, GREEN,
                      "Green increase");
            green_slider_value = new_value;
        });

        connect(greenSlider, &QSlider::valueChanged, [=, this](int value) {
            greenValueLabel->setText(
                QString("Green increase: %1").arg(greenSlider->value()));
        });

        // SLIDER 3: BLUE
        QHBoxLayout* blueLayout = new QHBoxLayout;
        QSlider* blueSlider = new QSlider(Qt::Horizontal);
        blueSlider->setRange(-255, 255);
        blueSlider->setValue(0);
        blueSlider->setMaximumWidth(300); // Set to appropriate value
        QLabel* blueMinLabel = new QLabel("-255");
        QLabel* blueMaxLabel = new QLabel("255");
        QLabel* blueValueLabel = new QLabel;
        blueValueLabel->setText(
            QString("Blue increase: %1").arg(blueSlider->value()));
        blueLayout->addWidget(blueMinLabel);

        blueLayout->addWidget(blueSlider);
        blueLayout->addWidget(blueMaxLabel);

        QVBoxLayout* blueVLayout = new QVBoxLayout;
        blueVLayout->addLayout(blueLayout);

        QHBoxLayout* blue_Layout = new QHBoxLayout;
        blue_Layout->addStretch(1); // add a stretchable empty space at left
        blue_Layout->addWidget(blueValueLabel); // add your label
        blue_Layout->addStretch(1); // add a stretchable empty space at right

        blueVLayout->addLayout(blue_Layout);

        checkBoxChannelLayout->addLayout(blueVLayout);

        blue_slider_value = 0;

        connect(blueSlider, &QSlider::sliderPressed, [=, this]() {
            qDebug() << "Initial value: " << blueSlider->value();
            blue_slider_value = blueSlider->value();
        });

        connect(blueSlider, &QSlider::sliderReleased, [=, this]() {
            qDebug() << "Final value: " << blueSlider->value();
            auto old_value = blue_slider_value;
            auto new_value = blueSlider->value();
            applySwap(tifo::increase_channel, new_value - old_value, BLUE,
                      "Blue increase");
            blue_slider_value = new_value;
        });

        connect(blueSlider, &QSlider::valueChanged, [=, this](int value) {
            blueValueLabel->setText(
                QString("Blue increase: %1").arg(blueSlider->value()));
        });

        connect(toggleChannelsButton, &QPushButton::clicked, [=]() {
            bool isVisible = channelsGroup->isVisible();
            channelsGroup->setVisible(!isVisible);
            if (isVisible)
            {
                toggleChannelsButton->setText("> Channels increasing");
            }
            else
            {
                toggleChannelsButton->setText("V Channels increasing");
            }
        });

        /**
         ** PROCESSING OPTIONS PART
         **/

        QPushButton* toggleOptionsButton = new QPushButton("> Options");
        optionsLayout->addWidget(toggleOptionsButton);

        QGroupBox* optionsGroup = new QGroupBox();
        optionsGroup->setMaximumHeight(200);
        optionsGroup->setCheckable(false); // Not checkable
        optionsGroup->setVisible(false); // Initially not visible
        optionsLayout->addWidget(optionsGroup);

        QVBoxLayout* checkBoxLayout = new QVBoxLayout;
        optionsGroup->setLayout(checkBoxLayout);

        // SLIDER 1: SATURATION
        QHBoxLayout* slider1Layout = new QHBoxLayout;
        QSlider* slider1 = new QSlider(Qt::Horizontal);
        slider1->setRange(-100, 100);
        slider1->setValue(0);
        slider1->setMaximumWidth(300); // Set to appropriate value
        QLabel* slider1MinLabel = new QLabel("-100");
        QLabel* slider1MaxLabel = new QLabel("100");
        QLabel* slider1ValueLabel = new QLabel;
        slider1ValueLabel->setText(
            QString("Saturation: %1").arg(slider1->value()));
        slider1Layout->addWidget(slider1MinLabel);

        slider1Layout->addWidget(slider1);
        slider1Layout->addWidget(slider1MaxLabel);

        QVBoxLayout* slider1VLayout = new QVBoxLayout;
        slider1VLayout->addLayout(slider1Layout);

        QHBoxLayout* label1Layout = new QHBoxLayout;
        label1Layout->addStretch(1); // add a stretchable empty space at left
        label1Layout->addWidget(slider1ValueLabel); // add your label
        label1Layout->addStretch(1); // add a stretchable empty space at right

        slider1VLayout->addLayout(label1Layout);

        checkBoxLayout->addLayout(slider1VLayout);

        slider1_value = 0;

        connect(slider1, &QSlider::sliderPressed, [=, this]() {
            qDebug() << "Initial value: " << slider1->value();
            // slider1_value = slider1->value();
        });

        connect(slider1, &QSlider::sliderReleased, [=, this]() {
            qDebug() << "Final value: " << slider1->value();
            auto old_value = slider1_value;
            auto new_value = slider1->value();
            applySlider(tifo::rgb_saturation, new_value - old_value,
                        "Saturation");
            slider1_value = new_value;
        });

        connect(slider1, &QSlider::valueChanged, [=, this](int value) {
            slider1ValueLabel->setText(
                QString("Saturation: %1%").arg(slider1->value()));
        });

        // SLIDER 2: CONTRAST
        QHBoxLayout* slider2Layout = new QHBoxLayout;
        QSlider* slider2 = new QSlider(Qt::Horizontal);
        slider2->setRange(0, 200);
        slider2->setValue(100);
        slider2->setMaximumWidth(300); // Set to appropriate value
        QLabel* slider2MinLabel = new QLabel("0%");
        QLabel* slider2MaxLabel = new QLabel("200%");
        QLabel* slider2ValueLabel = new QLabel;
        slider2Layout->addWidget(slider2MinLabel);
        slider2Layout->addWidget(slider2);
        slider2Layout->addWidget(slider2MaxLabel);
        checkBoxLayout->addLayout(slider2Layout);
        checkBoxLayout->addWidget(slider2ValueLabel);
        slider2ValueLabel->setText(
            QString("Contrast: %1%").arg(slider2->value()));

        QVBoxLayout* slider2VLayout = new QVBoxLayout;
        slider2VLayout->addLayout(slider2Layout);

        QHBoxLayout* label2Layout = new QHBoxLayout;
        label2Layout->addStretch(1);
        label2Layout->addWidget(slider2ValueLabel);
        label2Layout->addStretch(1);

        slider2VLayout->addLayout(label2Layout);

        checkBoxLayout->addLayout(slider2VLayout);

        slider2_value = 100;

        connect(slider2, &QSlider::sliderPressed, [=, this]() {
            qDebug() << "Initial value: " << slider2->value();
            slider2_value = slider2->value();
        });

        connect(slider2, &QSlider::sliderReleased, [=, this]() {
            qDebug() << "Final value: " << slider2->value();
            auto old_value = std::max(slider2_value, 1);
            auto new_value = slider2->value() * 100;
            applySlider(tifo::increase_contrast, (int)(new_value / old_value),
                        "Constrast");
            slider2_value = new_value;
        });

        connect(slider2, &QSlider::valueChanged, [=, this](int value) {
            slider2ValueLabel->setText(
                QString("Contrast: %1%").arg(slider2->value()));
        });

        // SLIDER 3: BLACK POINT
        QHBoxLayout* slider3Layout = new QHBoxLayout;
        QSlider* slider3 = new QSlider(Qt::Horizontal);
        slider3->setRange(0, 255);
        slider3->setValue(0);
        slider3->setMaximumWidth(300);
        QLabel* slider3MinLabel = new QLabel("0");
        QLabel* slider3MaxLabel = new QLabel("255");
        QLabel* slider3ValueLabel = new QLabel;
        slider3Layout->addWidget(slider3MinLabel);
        slider3Layout->addWidget(slider3);
        slider3Layout->addWidget(slider3MaxLabel);
        checkBoxLayout->addLayout(slider3Layout);
        checkBoxLayout->addWidget(slider3ValueLabel);
        slider3ValueLabel->setText(
            QString("Black Point: %1").arg(slider3->value()));

        // Create a new layout to hold the slider and the label
        QVBoxLayout* slider3VLayout = new QVBoxLayout;
        slider3VLayout->addLayout(slider3Layout);

        QHBoxLayout* label3Layout = new QHBoxLayout;
        label3Layout->addStretch(1); // add a stretchable empty space at left
        label3Layout->addWidget(slider3ValueLabel); // add your label
        label3Layout->addStretch(1); // add a stretchable empty space at right

        slider3VLayout->addLayout(label3Layout);

        checkBoxLayout->addLayout(slider3VLayout);

        slider3_value = 0;

        connect(slider3, &QSlider::sliderReleased, [=, this]() {
            qDebug() << "Final value: " << slider3->value();
            auto old_value = slider3_value;
            auto new_value = slider3->value();
            applySlider(tifo::adjust_black_point, new_value - old_value,
                        "Black Point");
            slider3_value = new_value;
        });

        connect(slider3, &QSlider::valueChanged, [=, this](int value) {
            slider3ValueLabel->setText(
                QString("Black Point: %1%").arg(slider3->value()));
        });

        connect(toggleOptionsButton, &QPushButton::clicked, [=]() {
            bool isVisible = optionsGroup->isVisible();
            optionsGroup->setVisible(!isVisible);
            if (isVisible)
            {
                toggleOptionsButton->setText("> Options");
            }
            else
            {
                toggleOptionsButton->setText("V Options");
            }
        });
    }

    void mousePressEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton)
        {
            QPoint pos = event->pos();
            qDebug() << "Mouse clicked at position" << pos;
        }

        // Pass event to base class to ensure it gets handled properly
        QWidget::mousePressEvent(event);
    }

public slots:
    void loadImage()
    {
        QString fileName = QFileDialog::getOpenFileName(
            this, "Open Image", "", "Image Files (*.png *.jpg *.bmp *.tga)");
        if (!fileName.isEmpty())
        {
            m_image.load(fileName);
            QPixmap pixmap = QPixmap::fromImage(m_image);
            m_imageLabel->setPixmap(pixmap);
            index = 0;
            images = std::vector<QImage>(1);
            images[0] = m_image;
            saveButton->setEnabled(true);
            backwardButton->setEnabled(true);
            forwardButton->setEnabled(true);
            originalButton->setEnabled(true);
            optionsWidget->setEnabled(true);
        }
    }

    void saveImage()
    {
        QString fileName = QFileDialog::getSaveFileName(
            this, "Save Image", "",
            "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)");
        if (!fileName.isEmpty())
        {
            m_image.save(fileName);
        }
    }

    void backwardImage()
    {
        qDebug() << index;

        if (index > 0 && index <= images.size() - 1)
        {
            index--;

            qDebug() << index;

            m_image = images[index];

            m_imageLabel->setPixmap(QPixmap::fromImage(m_image));
        }
    }

    void forwardImage()
    {
        qDebug() << index;

        if (index != images.size() - 1)
        {
            qDebug() << index;

            index++;

            m_image = images[index];

            m_imageLabel->setPixmap(QPixmap::fromImage(m_image));
        }
    }

    void originalImage()
    {
        index = 0;

        m_image = images[index];

        m_imageLabel->setPixmap(QPixmap::fromImage(m_image));

        images = std::vector<QImage>(1);

        images[0] = m_image;
    }

    void
    applySlider(const std::function<void(tifo::rgb24_image&, int)>& processing,
                int arg, const char* str)
    {
        qDebug() << arg;

        QElapsedTimer timer1;
        timer1.start();
        auto tmp = qimage_to_rgb(images[index]);
        QElapsedTimer timer2;
        timer2.start();

        processing(*tmp, arg);

        m_image = rgb_to_qimage(*tmp);

        m_imageLabel->setPixmap(QPixmap::fromImage(m_image));

        qDebug() << str << " execution time: " << timer2.elapsed() << "ms";

        index++;

        if (images.size() == index)
        {
            images.push_back(m_image);
        }
        else
        {
            images[index] = m_image;
        }

        qDebug() << "Whole " << str
                 << " process execution time: " << timer1.elapsed() << "ms";
    }

    void applyFilter(const std::function<void(tifo::rgb24_image&)>& filter,
                     const char* str)
    {
        QElapsedTimer timer1;
        timer1.start();
        auto tmp = qimage_to_rgb(images[index]);
        QElapsedTimer timer2;
        timer2.start();

        filter(*tmp);

        m_image = rgb_to_qimage(*tmp);

        m_imageLabel->setPixmap(QPixmap::fromImage(m_image));

        qDebug() << str << " filter execution time: " << timer2.elapsed()
                 << "ms";

        index++;

        if (images.size() == index)
        {
            images.push_back(m_image);
        }
        else
        {
            images[index] = m_image;
        }

        qDebug() << "Whole " << str
                 << " process execution time: " << timer1.elapsed() << "ms";
    }

    void
    applyGlow(const std::function<void(tifo::rgb24_image&, float, int)>& filter,
              float radius, int threshold, const char* str)
    {
        QElapsedTimer timer1;
        timer1.start();
        auto tmp = qimage_to_rgb(images[index]);
        QElapsedTimer timer2;
        timer2.start();

        filter(*tmp, radius, threshold);

        m_image = rgb_to_qimage(*tmp);

        m_imageLabel->setPixmap(QPixmap::fromImage(m_image));

        qDebug() << str << " filter execution time: " << timer2.elapsed()
                 << "ms";

        index++;

        if (images.size() == index)
        {
            images.push_back(m_image);
        }
        else
        {
            images[index] = m_image;
        }

        qDebug() << "Whole " << str
                 << " process execution time: " << timer1.elapsed() << "ms";
    }

    void
    applySwap(const std::function<void(tifo::rgb24_image&, int, int)>& filter,
              int channel1, int channel2, const char* str)
    {
        QElapsedTimer timer1;
        timer1.start();
        auto tmp = qimage_to_rgb(images[index]);
        QElapsedTimer timer2;
        timer2.start();

        filter(*tmp, channel1, channel2);

        m_image = rgb_to_qimage(*tmp);

        m_imageLabel->setPixmap(QPixmap::fromImage(m_image));

        qDebug() << str << " filter execution time: " << timer2.elapsed()
                 << "ms";

        index++;

        if (images.size() == index)
        {
            images.push_back(m_image);
        }
        else
        {
            images[index] = m_image;
        }

        qDebug() << "Whole " << str
                 << " process execution time: " << timer1.elapsed() << "ms";
    }

private:
    QImage m_image;
    ResizableImageLabel* m_imageLabel;
    std::vector<QImage> images;

    SquareButton* saveButton;
    SquareButton* backwardButton;
    SquareButton* forwardButton;
    SquareButton* originalButton;
    QWidget* optionsWidget;

    int index;

    int slider1_value;
    int slider2_value;
    int slider3_value;
    int red_slider_value;
    int green_slider_value;
    int blue_slider_value;

    std::vector<std::vector<int>> sliders_values;
};
