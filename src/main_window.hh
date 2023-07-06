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
#include "image_convert.hh"
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

        connect(glowRadiusSlider, &QSlider::valueChanged, [=]() {
            valueGlowRadius->setText(
                QString("Glow Radius: %1").arg(glowRadiusSlider->value()));
        });

        connect(glowRadiusSlider, &QSlider::sliderReleased,
                [=, this]() { glowRadius_value = glowRadiusSlider->value(); });

        connect(glowThresholdSlider, &QSlider::valueChanged, [=]() {
            valueGlowThreshold->setText(QString("Glow Threshold: %1")
                                            .arg(glowThresholdSlider->value()));
        });

        connect(glowThresholdSlider, &QSlider::sliderReleased, [=, this]() {
            glowThreshold_value = glowThresholdSlider->value();
        });

        glowRadiusLayout->addLayout(glowRadiusSliderLayout);
        glowRadiusLayout->addWidget(valueGlowRadius);

        glowThresholdLayout->addLayout(glowThresholdSliderLayout);
        glowThresholdLayout->addWidget(valueGlowThreshold);

        glowFilterLayout->addLayout(glowRadiusLayout);
        glowFilterLayout->addLayout(glowThresholdLayout);

        QPushButton* glowFilterButton =
            new QPushButton("Apply glow filter", this);
        connect(glowFilterButton, &QPushButton::clicked, this, [=, this]() {
            applyGlow(tifo::glow_filter, (float)glowRadius_value,
                      glowThreshold_value, "Glow");
        });
        glowFilterLayout->addWidget(glowFilterButton);

        filtersCheckBoxLayout->addLayout(glowFilterLayout);

        // GAUSSIAN BLUR

        QVBoxLayout* gaussianFilterLayout = new QVBoxLayout;
        QVBoxLayout* gaussianSizeLayout = new QVBoxLayout;
        QVBoxLayout* gaussianRadiusLayout = new QVBoxLayout;

        QHBoxLayout* gaussianSizeSliderLayout = new QHBoxLayout;
        QHBoxLayout* gaussianRadiusSliderLayout = new QHBoxLayout;

        QLabel* minGaussianSize = new QLabel("1");
        QLabel* maxGaussianSize = new QLabel("9");

        QLabel* minGaussianRadius = new QLabel("0");
        QLabel* maxGaussianRadius = new QLabel("100");

        QLabel* valueGaussianSize = new QLabel("Gaussian Size: 0");
        QLabel* valueGaussianRadius = new QLabel("Gaussian Radius: 0");

        QSlider* gaussianSizeSlider = new QSlider(Qt::Horizontal);
        QSlider* gaussianRadiusSlider = new QSlider(Qt::Horizontal);

        gaussianSizeSlider->setRange(1, 9);
        gaussianSizeSlider->setValue(3);
        gaussianSizeSlider->setMaximumWidth(300);

        gaussianRadiusSlider->setRange(0, 100);
        gaussianRadiusSlider->setValue(0);
        gaussianRadiusSlider->setMaximumWidth(300);

        gaussianSizeSliderLayout->addWidget(minGaussianSize);
        gaussianSizeSliderLayout->addWidget(gaussianSizeSlider);
        gaussianSizeSliderLayout->addWidget(maxGaussianSize);

        gaussianRadiusSliderLayout->addWidget(minGaussianRadius);
        gaussianRadiusSliderLayout->addWidget(gaussianRadiusSlider);
        gaussianRadiusSliderLayout->addWidget(maxGaussianRadius);

        connect(gaussianSizeSlider, &QSlider::valueChanged, [=]() {
            valueGaussianSize->setText(
                QString("Gaussian Size: %1").arg(gaussianSizeSlider->value()));
        });

        connect(gaussianSizeSlider, &QSlider::sliderReleased, [=, this]() {
            gaussianSize_value = gaussianSizeSlider->value();
        });

        connect(gaussianRadiusSlider, &QSlider::valueChanged, [=]() {
            valueGaussianRadius->setText(
                QString("Gaussian Radius: %1")
                    .arg(gaussianRadiusSlider->value()));
        });

        connect(gaussianRadiusSlider, &QSlider::sliderReleased, [=, this]() {
            gaussianRadius_value = gaussianRadiusSlider->value();
        });

        gaussianSizeLayout->addLayout(gaussianSizeSliderLayout);
        gaussianSizeLayout->addWidget(valueGaussianSize);

        gaussianRadiusLayout->addLayout(gaussianRadiusSliderLayout);
        gaussianRadiusLayout->addWidget(valueGaussianRadius);

        gaussianFilterLayout->addLayout(gaussianSizeLayout);
        gaussianFilterLayout->addLayout(gaussianRadiusLayout);

        QPushButton* gaussianFilterButton =
            new QPushButton("Apply Gaussian blur", this);
        connect(gaussianFilterButton, &QPushButton::clicked, this, [=, this]() {
            if (gaussianSize_value % 2 == 0)
                QMessageBox::information(this, tr("ERROR"),
                                         tr("The size must be odd"));
            else
            {
                applyGaussian(tifo::rgb_gaussian, gaussianSize_value,
                              (float)gaussianRadius_value, "Gaussian");
            }
        });
        gaussianFilterLayout->addWidget(gaussianFilterButton);

        filtersCheckBoxLayout->addLayout(gaussianFilterLayout);

        // SOBEL

        QVBoxLayout* sobelFilterLayout = new QVBoxLayout;

        QHBoxLayout* sobelSpaceLayout = new QHBoxLayout;

        sobelSpaceLayout->addStretch(1);
        QCheckBox* sobelRGBCheckBox = new QCheckBox("RGB", this);
        sobelSpaceLayout->addWidget(sobelRGBCheckBox);

        sobelSpaceLayout->addStretch(1);

        QCheckBox* sobelHSVCheckBox = new QCheckBox("HSV", this);
        sobelSpaceLayout->addWidget(sobelHSVCheckBox);

        sobelSpaceLayout->addStretch(1);

        QCheckBox* sobelYCrCbCheckBox = new QCheckBox("YCrCb", this);
        sobelSpaceLayout->addWidget(sobelYCrCbCheckBox);

        sobelSpaceLayout->addStretch(1);

        QCheckBox* sobelGrayCheckBox = new QCheckBox("GRAY", this);
        sobelSpaceLayout->addWidget(sobelGrayCheckBox);

        sobelSpaceLayout->addStretch(1);

        sobelFilterLayout->addLayout(sobelSpaceLayout);

        QPushButton* sobelFilterButton =
            new QPushButton("Apply sobel filter", this);
        connect(sobelFilterButton, &QPushButton::clicked, this, [=, this]() {
            if (sobelRGBCheckBox->isChecked())
            {
                applyFilter(tifo::sobel_rgb, "Sobel RGB");
            }
            else if (sobelHSVCheckBox->isChecked())
            {
                applyFilter(tifo::sobel_hsv, "Sobel HSV");
            }
            else if (sobelYCrCbCheckBox->isChecked())
            {
                applyFilter(tifo::sobel_yCrCb, "Sobel YCrCb");
            }
            else
            {
                applyFilter(tifo::sobel_gray, "Sobel GRAY");
            }
        });
        sobelFilterButton->setEnabled(false);
        sobelFilterLayout->addWidget(sobelFilterButton);

        auto lambdaFuncSobel = [sobelRGBCheckBox, sobelHSVCheckBox,
                                sobelYCrCbCheckBox, sobelGrayCheckBox,
                                sobelFilterButton]() {
            int totalChecked = sobelRGBCheckBox->isChecked()
                + sobelHSVCheckBox->isChecked()
                + sobelYCrCbCheckBox->isChecked()
                + sobelGrayCheckBox->isChecked();
            sobelFilterButton->setEnabled(totalChecked == 1);
        };

        connect(sobelRGBCheckBox, &QCheckBox::clicked, this, lambdaFuncSobel);
        connect(sobelHSVCheckBox, &QCheckBox::clicked, this, lambdaFuncSobel);
        connect(sobelYCrCbCheckBox, &QCheckBox::clicked, this, lambdaFuncSobel);
        connect(sobelGrayCheckBox, &QCheckBox::clicked, this, lambdaFuncSobel);

        filtersCheckBoxLayout->addLayout(sobelFilterLayout);

        // LAPLACIAN

        QVBoxLayout* laplacianFilterLayout = new QVBoxLayout;
        QVBoxLayout* laplacianKLayout = new QVBoxLayout;

        QHBoxLayout* laplacianKSliderLayout = new QHBoxLayout;

        QLabel* minLaplacianK = new QLabel("-1");
        QLabel* maxLaplacianK = new QLabel("1");

        QLabel* valueLaplacianK = new QLabel("Laplacian K: 0");

        QSlider* laplacianKSlider = new QSlider(Qt::Horizontal);

        laplacianKSlider->setRange(-100, 100);
        laplacianKSlider->setValue(0);
        laplacianKSlider->setMaximumWidth(300);

        laplacianKSliderLayout->addWidget(minLaplacianK);
        laplacianKSliderLayout->addWidget(laplacianKSlider);
        laplacianKSliderLayout->addWidget(maxLaplacianK);

        connect(laplacianKSlider, &QSlider::valueChanged, [=]() {
            valueLaplacianK->setText(
                QString("Laplacian K: %1")
                    .arg(laplacianKSlider->value() / 100.0));
        });

        connect(laplacianKSlider, &QSlider::sliderReleased,
                [=, this]() { laplacianK_value = laplacianKSlider->value(); });

        laplacianKLayout->addLayout(laplacianKSliderLayout);
        laplacianKLayout->addWidget(valueLaplacianK);

        laplacianFilterLayout->addLayout(laplacianKLayout);

        QHBoxLayout* laplacianSpaceLayout = new QHBoxLayout;

        laplacianSpaceLayout->addStretch(1);

        QCheckBox* laplacianRGBCheckBox = new QCheckBox("RGB", this);
        laplacianSpaceLayout->addWidget(laplacianRGBCheckBox);

        laplacianSpaceLayout->addStretch(1);

        QCheckBox* laplacianHSVCheckBox = new QCheckBox("HSV", this);
        laplacianSpaceLayout->addWidget(laplacianHSVCheckBox);

        laplacianSpaceLayout->addStretch(1);

        QCheckBox* laplacianYCrCbCheckBox = new QCheckBox("YCrCb", this);
        laplacianSpaceLayout->addWidget(laplacianYCrCbCheckBox);

        laplacianSpaceLayout->addStretch(1);

        QCheckBox* laplacianGrayCheckBox = new QCheckBox("GRAY", this);
        laplacianSpaceLayout->addWidget(laplacianGrayCheckBox);

        laplacianSpaceLayout->addStretch(1);

        laplacianFilterLayout->addLayout(laplacianSpaceLayout);

        QPushButton* laplacianFilterButton =
            new QPushButton("Apply laplacian filter", this);
        connect(laplacianFilterButton, &QPushButton::clicked, this,
                [=, this]() {
                    if (laplacianRGBCheckBox->isChecked())
                    {
                        applyFilterFloat(tifo::laplacien_filter_rgb,
                                         (float)laplacianK_value / 100,
                                         "Laplacian RGB");
                    }
                    else if (laplacianHSVCheckBox->isChecked())
                    {
                        applyFilterFloat(tifo::laplacien_filter_hsv,
                                         (float)laplacianK_value / 100,
                                         "Laplacian");
                    }
                    else if (laplacianYCrCbCheckBox->isChecked())
                    {
                        applyFilterFloat(tifo::laplacien_filter_yCrCb,
                                         (float)laplacianK_value / 100,
                                         "Laplacian");
                    }
                    else
                    {
                        applyFilterFloat(tifo::laplacian_gray,
                                         (float)laplacianK_value / 100,
                                         "Laplacian");
                    }
                });
        laplacianFilterButton->setEnabled(false);
        laplacianFilterLayout->addWidget(laplacianFilterButton);

        auto lambdaFuncLaplacian =
            [laplacianRGBCheckBox, laplacianHSVCheckBox, laplacianYCrCbCheckBox,
             laplacianGrayCheckBox, laplacianFilterButton]() {
                int totalChecked = laplacianRGBCheckBox->isChecked()
                    + laplacianHSVCheckBox->isChecked()
                    + laplacianYCrCbCheckBox->isChecked()
                    + laplacianGrayCheckBox->isChecked();
                laplacianFilterButton->setEnabled(totalChecked == 1);
            };

        connect(laplacianRGBCheckBox, &QCheckBox::clicked, this,
                lambdaFuncLaplacian);
        connect(laplacianHSVCheckBox, &QCheckBox::clicked, this,
                lambdaFuncLaplacian);
        connect(laplacianYCrCbCheckBox, &QCheckBox::clicked, this,
                lambdaFuncLaplacian);
        connect(laplacianGrayCheckBox, &QCheckBox::clicked, this,
                lambdaFuncLaplacian);

        filtersCheckBoxLayout->addLayout(laplacianFilterLayout);

        // HIDE / SHOW
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
         ** FLIP
         **/

        QPushButton* toggleFlipButton = new QPushButton("> Flip");
        optionsLayout->addWidget(toggleFlipButton);

        QGroupBox* flipBox = new QGroupBox;
        flipBox->setVisible(false);
        flipBox->setMaximumHeight(100);

        QVBoxLayout* flipLayout = new QVBoxLayout;

        QPushButton* horizontalFilterButton =
            new QPushButton("Horizontal Flip", this);
        connect(horizontalFilterButton, &QPushButton::clicked, this, [this]() {
            applyFilter(tifo::horizontal_flip, "Horizontal Flip");
        });
        flipLayout->addWidget(horizontalFilterButton);

        QPushButton* verticalFilterButton =
            new QPushButton("Vertical Flip", this);
        connect(verticalFilterButton, &QPushButton::clicked, this, [this]() {
            applyFilter(tifo::vertical_flip, "Vertical Flip");
        });
        flipLayout->addWidget(verticalFilterButton);

        flipBox->setLayout(flipLayout);
        optionsLayout->addWidget(flipBox);

        connect(toggleFlipButton, &QPushButton::clicked, [=]() {
            bool isVisible = flipBox->isVisible();
            flipBox->setVisible(!isVisible);
            if (isVisible)
            {
                toggleFlipButton->setText("> Flip");
            }
            else
            {
                toggleFlipButton->setText("V Flip");
            }
        });

        /**
         ** ROTATE PART
         **/

        QPushButton* toggleRotateButton = new QPushButton("> Rotate");
        optionsLayout->addWidget(toggleRotateButton);

        QGroupBox* rotateBox = new QGroupBox;
        rotateBox->setVisible(false);
        rotateBox->setMaximumHeight(200);

        QVBoxLayout* rotateLayout = new QVBoxLayout;

        QHBoxLayout* rotateSliderLayout = new QHBoxLayout;

        QLabel* minRotateK = new QLabel("-180°");
        QLabel* maxRotateK = new QLabel("180°");

        QLabel* valueRotateK = new QLabel("Angle: 0°");

        QSlider* rotateSlider = new QSlider(Qt::Horizontal);

        rotateSlider->setRange(-180, 180);
        rotateSlider->setValue(0);
        rotateSlider->setMaximumWidth(300);

        rotateSliderLayout->addWidget(minRotateK);
        rotateSliderLayout->addWidget(rotateSlider);
        rotateSliderLayout->addWidget(maxRotateK);

        connect(rotateSlider, &QSlider::valueChanged, [=]() {
            valueRotateK->setText(
                QString("Angle: %1°").arg(rotateSlider->value()));
        });

        connect(rotateSlider, &QSlider::sliderReleased,
                [=, this]() { rotate_value = rotateSlider->value(); });

        rotateLayout->addLayout(rotateSliderLayout);
        rotateLayout->addWidget(valueRotateK);

        rotateLayout->addLayout(rotateLayout);

        QPushButton* rotateButton = new QPushButton("Rotate", this);
        connect(rotateButton, &QPushButton::clicked, this, [this]() {
            applyRotate(tifo::rotate_image, rotate_value, "Rotate");
        });
        rotateLayout->addWidget(rotateButton);

        rotateBox->setLayout(rotateLayout);
        optionsLayout->addWidget(rotateBox);

        connect(toggleRotateButton, &QPushButton::clicked, [=]() {
            bool isVisible = rotateBox->isVisible();
            rotateBox->setVisible(!isVisible);
            if (isVisible)
            {
                toggleRotateButton->setText("> Rotate");
            }
            else
            {
                toggleRotateButton->setText("V Rotate");
            }
        });

        /**
         ** SWAP CHANNELS PART
         **/

        QPushButton* toggleChangeChannelsButton = new QPushButton("> RGB Swap");
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
                toggleChangeChannelsButton->setText("> RGB Swap");
            }
            else
            {
                toggleChangeChannelsButton->setText("V RGB Swap");
            }
        });

        /**
         ** RGB INCREASING
         **/

        QPushButton* toggleChannelsButton = new QPushButton("> RGB");
        optionsLayout->addWidget(toggleChannelsButton);

        QGroupBox* channelsGroup = new QGroupBox();
        channelsGroup->setMaximumHeight(400);
        channelsGroup->setCheckable(false); // Not checkable
        channelsGroup->setVisible(false); // Initially not visible
        optionsLayout->addWidget(channelsGroup);

        QVBoxLayout* checkBoxChannelLayout = new QVBoxLayout;
        channelsGroup->setLayout(checkBoxChannelLayout);

        // SLIDER 1: RED
        red_value = 0;

        QVBoxLayout* redLayout = new QVBoxLayout;

        QHBoxLayout* redSliderLayout = new QHBoxLayout;

        QLabel* minRed = new QLabel("-255");
        QLabel* maxRed = new QLabel("255");

        QLabel* redValue = new QLabel("Red: 0");

        QSlider* redSlider = new QSlider(Qt::Horizontal);

        redSlider->setRange(-255, 255);
        redSlider->setValue(0);
        redSlider->setMaximumWidth(300);

        redSliderLayout->addWidget(minRed);
        redSliderLayout->addWidget(redSlider);
        redSliderLayout->addWidget(maxRed);

        connect(redSlider, &QSlider::valueChanged, [=]() {
            redValue->setText(QString("Red: %1").arg(redSlider->value()));
        });

        redLayout->addLayout(redSliderLayout);
        redLayout->addWidget(redValue);

        QPushButton* redButton = new QPushButton("Apply red changes", this);
        connect(redButton, &QPushButton::clicked, this, [=, this]() {
            auto old_value = red_value;
            auto current_value = redSlider->value();
            applySwap(tifo::increase_channel, current_value - old_value, RED,
                      "Red");
            red_value = current_value;
        });
        redLayout->addWidget(redButton);

        checkBoxChannelLayout->addLayout(redLayout);

        // SLIDER 2: GREEN
        green_value = 0;

        QVBoxLayout* greenLayout = new QVBoxLayout;

        QHBoxLayout* greenSliderLayout = new QHBoxLayout;

        QLabel* minGreen = new QLabel("-255");
        QLabel* maxGreen = new QLabel("255");

        QLabel* greenValue = new QLabel("Green: 0");

        QSlider* greenSlider = new QSlider(Qt::Horizontal);

        greenSlider->setRange(-255, 255);
        greenSlider->setValue(0);
        greenSlider->setMaximumWidth(300);

        greenSliderLayout->addWidget(minGreen);
        greenSliderLayout->addWidget(greenSlider);
        greenSliderLayout->addWidget(maxGreen);

        connect(greenSlider, &QSlider::valueChanged, [=]() {
            greenValue->setText(QString("Green: %1").arg(greenSlider->value()));
        });

        greenLayout->addLayout(greenSliderLayout);
        greenLayout->addWidget(greenValue);

        QPushButton* greenButton = new QPushButton("Apply green changes", this);
        connect(greenButton, &QPushButton::clicked, this, [=, this]() {
            auto old_value = green_value;
            auto current_value = greenSlider->value();
            applySwap(tifo::increase_channel, current_value - old_value, GREEN,
                      "Green");
            green_value = current_value;
        });
        greenLayout->addWidget(greenButton);

        checkBoxChannelLayout->addLayout(greenLayout);

        // SLIDER 3: BLUE
        blue_value = 0;

        QVBoxLayout* blueLayout = new QVBoxLayout;

        QHBoxLayout* blueSliderLayout = new QHBoxLayout;

        QLabel* minBlue = new QLabel("-255");
        QLabel* maxBlue = new QLabel("255");

        QLabel* blueValue = new QLabel("Blue: 0");

        QSlider* blueSlider = new QSlider(Qt::Horizontal);

        blueSlider->setRange(-255, 255);
        blueSlider->setValue(0);
        blueSlider->setMaximumWidth(300);

        blueSliderLayout->addWidget(minBlue);
        blueSliderLayout->addWidget(blueSlider);
        blueSliderLayout->addWidget(maxBlue);

        connect(blueSlider, &QSlider::valueChanged, [=]() {
            blueValue->setText(QString("Blue: %1").arg(blueSlider->value()));
        });

        blueLayout->addLayout(blueSliderLayout);
        blueLayout->addWidget(blueValue);

        QPushButton* blueButton = new QPushButton("Apply blue changes", this);
        connect(blueButton, &QPushButton::clicked, this, [=, this]() {
            auto old_value = blue_value;
            auto current_value = blueSlider->value();
            applySwap(tifo::increase_channel, current_value - old_value, BLUE,
                      "Blue");
            blue_value = current_value;
        });
        blueLayout->addWidget(blueButton);

        checkBoxChannelLayout->addLayout(blueLayout);

        connect(toggleChannelsButton, &QPushButton::clicked, [=]() {
            bool isVisible = channelsGroup->isVisible();
            channelsGroup->setVisible(!isVisible);
            if (isVisible)
            {
                toggleChannelsButton->setText("> RGB");
            }
            else
            {
                toggleChannelsButton->setText("V RGB");
            }
        });

        /**
         ** HSV INCREASING
         **/

        QPushButton* toggleHSVButton = new QPushButton("> HSV");
        optionsLayout->addWidget(toggleHSVButton);

        QGroupBox* HSVGroup = new QGroupBox();
        HSVGroup->setMaximumHeight(400);
        HSVGroup->setCheckable(false); // Not checkable
        HSVGroup->setVisible(false); // Initially not visible
        optionsLayout->addWidget(HSVGroup);

        QVBoxLayout* checkBoxHSVLayout = new QVBoxLayout;
        HSVGroup->setLayout(checkBoxHSVLayout);

        // SLIDER 1: HUE
        hue_value = 0;

        QVBoxLayout* hueLayout = new QVBoxLayout;

        QHBoxLayout* hueSliderLayout = new QHBoxLayout;

        QLabel* minHue = new QLabel("-360");
        QLabel* maxHue = new QLabel("360");

        QLabel* hueValue = new QLabel("Hue: 0");

        QSlider* hueSlider = new QSlider(Qt::Horizontal);

        hueSlider->setRange(-360, 360);
        hueSlider->setValue(0);
        hueSlider->setMaximumWidth(300);

        hueSliderLayout->addWidget(minHue);
        hueSliderLayout->addWidget(hueSlider);
        hueSliderLayout->addWidget(maxHue);

        connect(hueSlider, &QSlider::valueChanged, [=]() {
            hueValue->setText(QString("Hue: %1").arg(hueSlider->value()));
        });

        hueLayout->addLayout(hueSliderLayout);
        hueLayout->addWidget(hueValue);

        QPushButton* hueButton = new QPushButton("Apply hue changes", this);
        connect(hueButton, &QPushButton::clicked, this, [=, this]() {
            auto old_value = hue_value;
            auto current_value = hueSlider->value();
            applySlider(tifo::rgb_hue, current_value - old_value, "Hue");
            hue_value = current_value;
        });
        hueLayout->addWidget(hueButton);

        checkBoxHSVLayout->addLayout(hueLayout);

        // SLIDER 2: SATURATION
        saturation_value = 0;

        QVBoxLayout* saturationLayout = new QVBoxLayout;

        QHBoxLayout* saturationSliderLayout = new QHBoxLayout;

        QLabel* minSaturation = new QLabel("-100");
        QLabel* maxSaturation = new QLabel("100");

        QLabel* saturationValue = new QLabel("Saturation: 0");

        QSlider* saturationSlider = new QSlider(Qt::Horizontal);

        saturationSlider->setRange(-100, 100);
        saturationSlider->setValue(0);
        saturationSlider->setMaximumWidth(300);

        saturationSliderLayout->addWidget(minSaturation);
        saturationSliderLayout->addWidget(saturationSlider);
        saturationSliderLayout->addWidget(maxSaturation);

        connect(saturationSlider, &QSlider::valueChanged, [=]() {
            saturationValue->setText(
                QString("Saturation: %1").arg(saturationSlider->value()));
        });

        saturationLayout->addLayout(saturationSliderLayout);
        saturationLayout->addWidget(saturationValue);

        QPushButton* saturationButton =
            new QPushButton("Apply saturation changes", this);
        connect(saturationButton, &QPushButton::clicked, this, [=, this]() {
            auto old_value = saturation_value;
            auto current_value = saturationSlider->value();
            applySlider(tifo::rgb_saturation, current_value - old_value,
                        "Saturation");
            saturation_value = current_value;
        });
        saturationLayout->addWidget(saturationButton);

        checkBoxHSVLayout->addLayout(saturationLayout);

        // SLIDER 3: VALUE
        value_value = 0;

        QVBoxLayout* valueLayout = new QVBoxLayout;

        QHBoxLayout* valueSliderLayout = new QHBoxLayout;

        QLabel* minValue = new QLabel("-100");
        QLabel* maxValue = new QLabel("100");

        QLabel* valueValue = new QLabel("Value: 0");

        QSlider* valueSlider = new QSlider(Qt::Horizontal);

        valueSlider->setRange(-100, 100);
        valueSlider->setValue(0);
        valueSlider->setMaximumWidth(300);

        valueSliderLayout->addWidget(minValue);
        valueSliderLayout->addWidget(valueSlider);
        valueSliderLayout->addWidget(maxValue);

        connect(valueSlider, &QSlider::valueChanged, [=]() {
            valueValue->setText(QString("Value: %1").arg(valueSlider->value()));
        });

        valueLayout->addLayout(valueSliderLayout);
        valueLayout->addWidget(valueValue);

        QPushButton* valueButton = new QPushButton("Apply value changes", this);
        connect(valueButton, &QPushButton::clicked, this, [=, this]() {
            auto old_value = value_value;
            auto current_value = valueSlider->value();
            applySlider(tifo::rgb_value, current_value - old_value, "Value");
            value_value = current_value;
        });
        valueLayout->addWidget(valueButton);

        checkBoxHSVLayout->addLayout(valueLayout);

        connect(toggleHSVButton, &QPushButton::clicked, [=]() {
            bool isVisible = HSVGroup->isVisible();
            HSVGroup->setVisible(!isVisible);
            if (isVisible)
            {
                toggleHSVButton->setText("> HSV");
            }
            else
            {
                toggleHSVButton->setText("V HSV");
            }
        });

        /**
         ** YCrCb INCREASING
         **/

        QPushButton* toggleYCbCrButton = new QPushButton("> YCrCb");
        optionsLayout->addWidget(toggleYCbCrButton);

        QGroupBox* YCbCrGroup = new QGroupBox();
        YCbCrGroup->setMaximumHeight(400);
        YCbCrGroup->setCheckable(false); // Not checkable
        YCbCrGroup->setVisible(false); // Initially not visible
        optionsLayout->addWidget(YCbCrGroup);

        QVBoxLayout* checkBoxYCbCrLayout = new QVBoxLayout;
        YCbCrGroup->setLayout(checkBoxYCbCrLayout);

        // SLIDER 1: Y
        y_value = 0;

        QVBoxLayout* yLayout = new QVBoxLayout;

        QHBoxLayout* ySliderLayout = new QHBoxLayout;

        QLabel* minY = new QLabel("-255");
        QLabel* maxY = new QLabel("255");

        QLabel* yValue = new QLabel("Y: 0");

        QSlider* ySlider = new QSlider(Qt::Horizontal);

        ySlider->setRange(-255, 255);
        ySlider->setValue(0);
        ySlider->setMaximumWidth(300);

        ySliderLayout->addWidget(minY);
        ySliderLayout->addWidget(ySlider);
        ySliderLayout->addWidget(maxY);

        connect(ySlider, &QSlider::valueChanged, [=]() {
            yValue->setText(QString("Y: %1").arg(ySlider->value()));
        });

        yLayout->addLayout(ySliderLayout);
        yLayout->addWidget(yValue);

        QPushButton* yButton = new QPushButton("Apply y changes", this);
        connect(yButton, &QPushButton::clicked, this, [=, this]() {
            auto old_value = y_value;
            auto current_value = ySlider->value();
            applySwap(tifo::yCrCb_increase_channel, current_value - old_value,
                      RED, "Y");
            y_value = current_value;
        });
        yLayout->addWidget(yButton);

        checkBoxYCbCrLayout->addLayout(yLayout);

        // SLIDER 2: GREEN
        cr_value = 0;

        QVBoxLayout* crLayout = new QVBoxLayout;

        QHBoxLayout* crSliderLayout = new QHBoxLayout;

        QLabel* minCr = new QLabel("-255");
        QLabel* maxCr = new QLabel("255");

        QLabel* crValue = new QLabel("Cr: 0");

        QSlider* crSlider = new QSlider(Qt::Horizontal);

        crSlider->setRange(-255, 255);
        crSlider->setValue(0);
        crSlider->setMaximumWidth(300);

        crSliderLayout->addWidget(minCr);
        crSliderLayout->addWidget(crSlider);
        crSliderLayout->addWidget(maxCr);

        connect(crSlider, &QSlider::valueChanged, [=]() {
            crValue->setText(QString("Cr: %1").arg(crSlider->value()));
        });

        crLayout->addLayout(crSliderLayout);
        crLayout->addWidget(crValue);

        QPushButton* crButton = new QPushButton("Apply cr changes", this);
        connect(crButton, &QPushButton::clicked, this, [=, this]() {
            auto old_value = cr_value;
            auto current_value = crSlider->value();
            applySwap(tifo::yCrCb_increase_channel, current_value - old_value,
                      GREEN, "Cr");
            cr_value = current_value;
        });
        crLayout->addWidget(crButton);

        checkBoxYCbCrLayout->addLayout(crLayout);

        // SLIDER 3: BLUE
        cb_value = 0;

        QVBoxLayout* cbLayout = new QVBoxLayout;

        QHBoxLayout* cbSliderLayout = new QHBoxLayout;

        QLabel* minCb = new QLabel("-255");
        QLabel* maxCb = new QLabel("255");

        QLabel* cbValue = new QLabel("Cb: 0");

        QSlider* cbSlider = new QSlider(Qt::Horizontal);

        cbSlider->setRange(-255, 255);
        cbSlider->setValue(0);
        cbSlider->setMaximumWidth(300);

        cbSliderLayout->addWidget(minCb);
        cbSliderLayout->addWidget(cbSlider);
        cbSliderLayout->addWidget(maxCb);

        connect(cbSlider, &QSlider::valueChanged, [=]() {
            cbValue->setText(QString("Cb: %1").arg(cbSlider->value()));
        });

        cbLayout->addLayout(cbSliderLayout);
        cbLayout->addWidget(cbValue);

        QPushButton* cbButton = new QPushButton("Apply cb changes", this);
        connect(cbButton, &QPushButton::clicked, this, [=, this]() {
            auto old_value = cb_value;
            auto current_value = cbSlider->value();
            applySwap(tifo::yCrCb_increase_channel, current_value - old_value,
                      BLUE, "Cb");
            cb_value = current_value;
        });
        cbLayout->addWidget(cbButton);

        checkBoxYCbCrLayout->addLayout(cbLayout);

        connect(toggleYCbCrButton, &QPushButton::clicked, [=]() {
            bool isVisible = YCbCrGroup->isVisible();
            YCbCrGroup->setVisible(!isVisible);
            if (isVisible)
            {
                toggleYCbCrButton->setText("> YCrCb");
            }
            else
            {
                toggleYCbCrButton->setText("V YCrCb");
            }
        });

        /**
         ** PROCESSING OPTIONS PART
         **/

        QPushButton* toggleOptionsButton = new QPushButton("> Options");
        optionsLayout->addWidget(toggleOptionsButton);

        QGroupBox* optionsGroup = new QGroupBox();
        optionsGroup->setMaximumHeight(500);
        optionsGroup->setCheckable(false); // Not checkable
        optionsGroup->setVisible(false); // Initially not visible
        optionsLayout->addWidget(optionsGroup);

        QVBoxLayout* checkBoxLayout = new QVBoxLayout;
        optionsGroup->setLayout(checkBoxLayout);

        // SLIDER 1: CONTRAST
        contrast_value = 100;

        QVBoxLayout* contrastLayout = new QVBoxLayout;

        QHBoxLayout* contrastSliderLayout = new QHBoxLayout;

        QLabel* minContrast = new QLabel("0%");
        QLabel* maxContrast = new QLabel("200%");

        QLabel* contrastValue = new QLabel("Contrast: 100%");

        QSlider* contrastSlider = new QSlider(Qt::Horizontal);

        contrastSlider->setRange(0, 200);
        contrastSlider->setValue(100);
        contrastSlider->setMaximumWidth(300);

        contrastSliderLayout->addWidget(minContrast);
        contrastSliderLayout->addWidget(contrastSlider);
        contrastSliderLayout->addWidget(maxContrast);

        connect(contrastSlider, &QSlider::valueChanged, [=]() {
            contrastValue->setText(
                QString("Contrast: %1%").arg(contrastSlider->value()));
        });

        contrastLayout->addLayout(contrastSliderLayout);
        contrastLayout->addWidget(contrastValue);

        QPushButton* contrastButton =
            new QPushButton("Apply contrast changes", this);
        connect(contrastButton, &QPushButton::clicked, this, [=, this]() {
            auto old_value = std::max(contrast_value, 1);
            auto current_value = contrastSlider->value();
            applySlider(tifo::increase_contrast,
                        (int)((current_value * 100) / old_value), "Contrast");
            contrast_value = current_value;
        });
        contrastLayout->addWidget(contrastButton);

        checkBoxLayout->addLayout(contrastLayout);

        // SLIDER 2: BLACK POINT
        blackPoint_value = 0;

        QVBoxLayout* blackPointLayout = new QVBoxLayout;

        QHBoxLayout* blackPointSliderLayout = new QHBoxLayout;

        QLabel* minBlackPoint = new QLabel("0");
        QLabel* maxBlackPoint = new QLabel("255");

        QLabel* blackPointValue = new QLabel("BlackPoint: 0");

        QSlider* blackPointSlider = new QSlider(Qt::Horizontal);

        blackPointSlider->setRange(0, 255);
        blackPointSlider->setValue(0);
        blackPointSlider->setMaximumWidth(300);

        blackPointSliderLayout->addWidget(minBlackPoint);
        blackPointSliderLayout->addWidget(blackPointSlider);
        blackPointSliderLayout->addWidget(maxBlackPoint);

        connect(blackPointSlider, &QSlider::valueChanged, [=]() {
            blackPointValue->setText(
                QString("BlackPoint: %1").arg(blackPointSlider->value()));
        });

        blackPointLayout->addLayout(blackPointSliderLayout);
        blackPointLayout->addWidget(blackPointValue);

        QPushButton* blackPointButton =
            new QPushButton("Apply blackPoint changes", this);
        connect(blackPointButton, &QPushButton::clicked, this, [=, this]() {
            auto old_value = blackPoint_value;
            auto current_value = blackPointSlider->value();
            applySlider(tifo::adjust_black_point, current_value - old_value,
                        "BlackPoint");
            blackPoint_value = current_value;
        });
        blackPointLayout->addWidget(blackPointButton);

        checkBoxLayout->addLayout(blackPointLayout);

        // SLIDER 3: VIGNETTE

        vignette_value = 0;

        QVBoxLayout* vignetteLayout = new QVBoxLayout;

        QHBoxLayout* vignetteSliderLayout = new QHBoxLayout;

        QLabel* minVignette = new QLabel("-100");
        QLabel* maxVignette = new QLabel("100");

        QLabel* vignetteValue = new QLabel("Vignette: 0");

        QSlider* vignetteSlider = new QSlider(Qt::Horizontal);

        vignetteSlider->setRange(-100, 100);
        vignetteSlider->setValue(0);
        vignetteSlider->setMaximumWidth(300);

        vignetteSliderLayout->addWidget(minVignette);
        vignetteSliderLayout->addWidget(vignetteSlider);
        vignetteSliderLayout->addWidget(maxVignette);

        connect(vignetteSlider, &QSlider::valueChanged, [=]() {
            vignetteValue->setText(
                QString("Vignette: %1").arg(vignetteSlider->value()));
        });

        vignetteLayout->addLayout(vignetteSliderLayout);
        vignetteLayout->addWidget(vignetteValue);

        QPushButton* vignetteButton =
            new QPushButton("Apply vignette changes", this);
        connect(vignetteButton, &QPushButton::clicked, this, [=, this]() {
            auto old_value = vignette_value;
            auto current_value = vignetteSlider->value();
            applySlider(tifo::add_vignette, current_value - old_value,
                        "Vignette");
            vignette_value = current_value;
        });
        vignetteLayout->addWidget(vignetteButton);

        checkBoxLayout->addLayout(vignetteLayout);

        // SLIDER 4: GRAIN

        grain_value = 0;

        QVBoxLayout* grainLayout = new QVBoxLayout;

        QHBoxLayout* grainSliderLayout = new QHBoxLayout;

        QLabel* minGrain = new QLabel("0");
        QLabel* maxGrain = new QLabel("100");

        QLabel* grainValue = new QLabel("Grain: 0");

        QSlider* grainSlider = new QSlider(Qt::Horizontal);

        grainSlider->setRange(0, 100);
        grainSlider->setValue(0);
        grainSlider->setMaximumWidth(300);

        grainSliderLayout->addWidget(minGrain);
        grainSliderLayout->addWidget(grainSlider);
        grainSliderLayout->addWidget(maxGrain);

        connect(grainSlider, &QSlider::valueChanged, [=]() {
            grainValue->setText(QString("Grain: %1").arg(grainSlider->value()));
        });

        grainLayout->addLayout(grainSliderLayout);
        grainLayout->addWidget(grainValue);

        QPushButton* grainButton = new QPushButton("Apply grain changes", this);
        connect(grainButton, &QPushButton::clicked, this, [=, this]() {
            auto current_value = grainSlider->value();
            applySlider(tifo::apply_argentique_grain, current_value, "Grain");
            grain_value = current_value;
        });
        grainLayout->addWidget(grainButton);

        checkBoxLayout->addLayout(grainLayout);

        // SHOW OPTIONS
        connect(toggleOptionsButton, &QPushButton::clicked, [=]() {
            bool isVisible = optionsGroup->isVisible();
            optionsGroup->setVisible(!isVisible);
            if (isVisible)
            {
                toggleOptionsButton->setText("> Other");
            }
            else
            {
                toggleOptionsButton->setText("V Other");
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

    void applyFilterFloat(
        const std::function<void(tifo::rgb24_image&, float)>& processing,
        float arg, const char* str)
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

    void applyGaussian(
        const std::function<void(tifo::rgb24_image&, int, float)>& filter,
        int size, float radius, const char* str)
    {
        qDebug() << "radius = " << radius << ", size = " << size;
        QElapsedTimer timer1;
        timer1.start();
        auto tmp = qimage_to_rgb(images[index]);
        QElapsedTimer timer2;
        timer2.start();

        filter(*tmp, size, radius);

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
        qDebug() << "radius = " << radius << ", threshold = " << threshold;
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

    void applyRotate(const std::function<tifo::rgb24_image*(tifo::rgb24_image&,
                                                            int)>& processing,
                     int arg, const char* str)
    {
        qDebug() << arg;

        QElapsedTimer timer1;
        timer1.start();
        auto tmp = qimage_to_rgb(images[index]);
        QElapsedTimer timer2;
        timer2.start();

        auto new_image = processing(*tmp, arg);

        m_image = rgb_to_qimage(*new_image);

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

    int hue_value;
    int saturation_value;
    int value_value;

    int y_value;
    int cr_value;
    int cb_value;

    int contrast_value;
    int blackPoint_value;

    int vignette_value;
    int grain_value;

    int red_value;
    int green_value;
    int blue_value;

    int glowRadius_value;
    int glowThreshold_value;

    int gaussianRadius_value;
    int gaussianSize_value;

    int laplacianK_value;
    int rotate_value;

    std::vector<std::vector<int>> sliders_values;
};
