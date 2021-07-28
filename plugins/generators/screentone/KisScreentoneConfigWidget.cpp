/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoColor.h>
#include <filter/kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>
#include <kis_signals_blocker.h>
#include <kis_generator_registry.h>
#include <KisViewManager.h>
#include <KoUnit.h>

#include "KisScreentoneConfigWidget.h"
#include "KisScreentoneScreentoneFunctions.h"
#include "KisScreentoneGeneratorConfiguration.h"

static const QString pixelsInchSuffix(i18n(" pixels/inch"));
static const QString pixelsCentimeterSuffix(i18n(" pixels/cm"));
static const QString linesInchSuffix(i18n(" lines/inch"));
static const QString linesCentimeterSuffix(i18n(" lines/cm"));

KisScreentoneConfigWidget::KisScreentoneConfigWidget(QWidget* parent, const KoColorSpace *cs)
    : KisConfigWidget(parent)
    , m_view(nullptr)
    , m_colorSpace(cs)
{
    m_ui.setupUi(this);

    setupPatternComboBox();
    setupShapeComboBox();
    setupInterpolationComboBox();

    m_ui.sliderForegroundOpacity->setRange(0, 100);
    m_ui.sliderForegroundOpacity->setPrefix(i18n("Opacity: "));
    m_ui.sliderForegroundOpacity->setSuffix(i18n("%"));
    m_ui.sliderBackgroundOpacity->setRange(0, 100);
    m_ui.sliderBackgroundOpacity->setPrefix(i18n("Opacity: "));
    m_ui.sliderBackgroundOpacity->setSuffix(i18n("%"));
    m_ui.sliderBrightness->setRange(0.0, 100.0, 2);
    m_ui.sliderBrightness->setSingleStep(1.0);
    m_ui.sliderBrightness->setSuffix(i18n("%"));
    m_ui.sliderContrast->setRange(0.0, 100.0, 2);
    m_ui.sliderContrast->setSingleStep(1.0);
    m_ui.sliderContrast->setSuffix(i18n("%"));

    m_ui.buttonSimpleTransformation->setGroupPosition(KoGroupButton::GroupLeft);
    m_ui.buttonAdvancedTransformation->setGroupPosition(KoGroupButton::GroupRight);
    m_ui.sliderResolution->setRange(1.0, 9999.0, 2);
    m_ui.sliderResolution->setSoftRange(72.0, 600.0);
    m_ui.sliderFrequencyX->setRange(0.01, 1000.0, 2);
    m_ui.sliderFrequencyX->setSoftRange(1.0, 100.0);
    m_ui.sliderFrequencyX->setPrefix(i18n("X: "));
    m_ui.sliderFrequencyX->setSingleStep(1.0);
    m_ui.sliderFrequencyY->setRange(0.01, 1000.0, 2);
    m_ui.sliderFrequencyY->setSoftRange(1.0, 100.0);
    m_ui.sliderFrequencyY->setPrefix(i18n("Y: "));
    m_ui.sliderFrequencyY->setSingleStep(1.0);
    slot_comboBoxUnits_currentIndexChanged(0);
    m_ui.sliderPositionX->setRange(-1000.0, 1000.0, 2);
    m_ui.sliderPositionX->setSoftRange(-100.0, 100.0);
    m_ui.sliderPositionX->setPrefix(i18n("X: "));
    m_ui.sliderPositionX->setSuffix(i18n(" px"));
    m_ui.sliderPositionX->setSingleStep(1.0);
    m_ui.sliderPositionY->setRange(-1000.0, 1000.0, 2);
    m_ui.sliderPositionY->setSoftRange(-100.0, 100.0);
    m_ui.sliderPositionY->setPrefix(i18n("Y: "));
    m_ui.sliderPositionY->setSuffix(i18n(" px"));
    m_ui.sliderPositionY->setSingleStep(1.0);
    m_ui.sliderSizeX->setRange(1.0, 1000.0, 2);
    m_ui.sliderSizeX->setSoftRange(1.0, 100.0);
    m_ui.sliderSizeX->setPrefix(i18n("X: "));
    m_ui.sliderSizeX->setSuffix(i18n(" px"));
    m_ui.sliderSizeX->setSingleStep(1.0);
    m_ui.sliderSizeX->setExponentRatio(4.32);
    m_ui.sliderSizeY->setRange(1.0, 1000.0, 2);
    m_ui.sliderSizeY->setSoftRange(1.0, 100.0);
    m_ui.sliderSizeY->setPrefix(i18n("Y: "));
    m_ui.sliderSizeY->setSuffix(i18n(" px"));
    m_ui.sliderSizeY->setSingleStep(1.0);
    m_ui.sliderSizeY->setExponentRatio(4.32);
    m_ui.sliderShearX->setRange(-10.0, 10.0, 2);
    m_ui.sliderShearX->setSoftRange(-2.0, 2.0);
    m_ui.sliderShearX->setPrefix(i18n("X: "));
    m_ui.sliderShearX->setSingleStep(0.1);
    m_ui.sliderShearY->setRange(-10.0, 10.0, 2);
    m_ui.sliderShearY->setSoftRange(-2.0, 2.0);
    m_ui.sliderShearY->setPrefix(i18n("Y: "));
    m_ui.sliderShearY->setSingleStep(0.1);
    slot_buttonSimpleTransformation_toggled(true);

    connect(m_ui.comboBoxPattern, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_comboBoxPattern_currentIndexChanged(int)));
    connect(m_ui.comboBoxShape, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_comboBoxShape_currentIndexChanged(int)));
    connect(m_ui.comboBoxInterpolation, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    
    connect(m_ui.buttonForegroundColor, SIGNAL(changed(const KoColor&)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderForegroundOpacity, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.buttonBackgroundColor, SIGNAL(changed(const KoColor&)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderBackgroundOpacity, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.checkBoxInvert, SIGNAL(toggled(bool)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderBrightness, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderContrast, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    
    connect(m_ui.buttonSimpleTransformation, SIGNAL(toggled(bool)), this, SLOT(slot_buttonSimpleTransformation_toggled(bool)));
    connect(m_ui.buttonAdvancedTransformation, SIGNAL(toggled(bool)), this, SLOT(slot_buttonAdvancedTransformation_toggled(bool)));
    connect(m_ui.comboBoxUnits, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_comboBoxUnits_currentIndexChanged(int)));
    connect(m_ui.sliderResolution, SIGNAL(valueChanged(qreal)), this, SLOT(slot_setAdvancedFromSimpleTransformation()));
    connect(m_ui.sliderResolution, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.buttonResolutionFromImage, SIGNAL(clicked()), this, SLOT(slot_buttonResolutionFromImage_clicked()));
    connect(m_ui.sliderFrequencyX, SIGNAL(valueChanged(qreal)), this, SLOT(slot_sliderFrequencyX_valueChanged(qreal)));
    connect(m_ui.sliderFrequencyY, SIGNAL(valueChanged(qreal)), this, SLOT(slot_sliderFrequencyY_valueChanged(qreal)));
    connect(m_ui.buttonConstrainFrequency, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slot_buttonConstrainFrequency_keepAspectRatioChanged(bool)));
    connect(m_ui.sliderPositionX, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderPositionY, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderSizeX, SIGNAL(valueChanged(qreal)), this, SLOT(slot_sliderSizeX_valueChanged(qreal)));
    connect(m_ui.sliderSizeY, SIGNAL(valueChanged(qreal)), this, SLOT(slot_sliderSizeY_valueChanged(qreal)));
    connect(m_ui.buttonConstrainSize, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slot_buttonConstrainSize_keepAspectRatioChanged(bool)));
    connect(m_ui.sliderShearX, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderShearY, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.angleSelectorRotation, SIGNAL(angleChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
}

KisScreentoneConfigWidget::~KisScreentoneConfigWidget()
{}

void KisScreentoneConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    const KisScreentoneGeneratorConfiguration *generatorConfig =
        dynamic_cast<const KisScreentoneGeneratorConfiguration*>(config.data());
        
    // The double slider spin boxes and the color buttons emit signals
    // when their value is set via code so we block signals here to 
    // prevent multiple sigConfigurationUpdated being called.
    // After the widgets are set up, unblock and emit sigConfigurationUpdated
    // just once 
    {
        KisSignalsBlocker blocker1(m_ui.buttonForegroundColor, m_ui.sliderForegroundOpacity,
                                   m_ui.buttonBackgroundColor, m_ui.sliderBackgroundOpacity,
                                   m_ui.sliderBrightness, m_ui.sliderContrast);
        KisSignalsBlocker blocker2(m_ui.checkBoxInvert, m_ui.comboBoxUnits,
                                   m_ui.sliderResolution, m_ui.buttonConstrainFrequency,
                                   m_ui.sliderFrequencyX, m_ui.sliderFrequencyY);
        KisSignalsBlocker blocker3(m_ui.sliderPositionX, m_ui.sliderPositionY,
                                   m_ui.sliderSizeX, m_ui.sliderSizeY,
                                   m_ui.sliderShearX, m_ui.sliderShearY);
        KisSignalsBlocker blocker4(m_ui.buttonConstrainSize, m_ui.angleSelectorRotation,
                                   m_ui.buttonSimpleTransformation, m_ui.buttonAdvancedTransformation);
        KisSignalsBlocker blocker5(this);

        m_ui.comboBoxPattern->setCurrentIndex(generatorConfig->pattern());
        m_ui.comboBoxShape->setCurrentIndex(shapeToComboIndex(generatorConfig->pattern(), generatorConfig->shape()));
        m_ui.comboBoxInterpolation->setCurrentIndex(generatorConfig->interpolation());
        
        KoColor c;
        c = generatorConfig->foregroundColor();
        c.convertTo(m_colorSpace);
        c.setOpacity(1.0);
        m_ui.buttonForegroundColor->setColor(c);
        m_ui.sliderForegroundOpacity->setValue(generatorConfig->foregroundOpacity());
        c = generatorConfig->backgroundColor();
        c.convertTo(m_colorSpace);
        c.setOpacity(1.0);
        m_ui.buttonBackgroundColor->setColor(c);
        m_ui.sliderBackgroundOpacity->setValue(generatorConfig->backgroundOpacity());
        m_ui.checkBoxInvert->setChecked(generatorConfig->invert());
        m_ui.sliderBrightness->setValue(generatorConfig->brightness());
        m_ui.sliderContrast->setValue(generatorConfig->contrast());

        m_ui.comboBoxUnits->setCurrentIndex(generatorConfig->units());
        m_ui.sliderResolution->setValue(generatorConfig->resolution());
        m_ui.buttonConstrainFrequency->setKeepAspectRatio(generatorConfig->constrainFrequency());
        m_ui.sliderFrequencyX->setValue(generatorConfig->frequencyX());
        // Set the frequency y slider to the frequency y value only if the frequency is not constrained
        if (m_ui.buttonConstrainFrequency->keepAspectRatio()) {
            m_ui.sliderFrequencyY->setValue(generatorConfig->frequencyX());
        } else {
            m_ui.sliderFrequencyY->setValue(generatorConfig->frequencyY());
        }
        m_ui.sliderPositionX->setValue(generatorConfig->positionX());
        m_ui.sliderPositionY->setValue(generatorConfig->positionY());
        m_ui.buttonConstrainSize->setKeepAspectRatio(generatorConfig->constrainSize());
        m_ui.sliderSizeX->setValue(generatorConfig->sizeX());
        // Set the size y slider to the sithe y value only if the size must not be squared
        if (m_ui.buttonConstrainSize->keepAspectRatio()) {
            m_ui.sliderSizeY->setValue(generatorConfig->sizeX());
        } else {
            m_ui.sliderSizeY->setValue(generatorConfig->sizeY());
        }
        m_ui.sliderShearX->setValue(generatorConfig->shearX());
        m_ui.sliderShearY->setValue(generatorConfig->shearY());
        m_ui.angleSelectorRotation->setAngle(generatorConfig->rotation());
        // We set here the fallback value to "advanced" if the "transformation_mode"
        // property is not present, which means the screentone was made with
        // previous versions of krita
        if (generatorConfig->transformationMode() == KisScreentoneTransformationMode_Advanced) {
            m_ui.buttonAdvancedTransformation->setChecked(true);
            slot_setSimpleFromAdvancedTransformation();
            slot_buttonAdvancedTransformation_toggled(true);
        } else {
            m_ui.buttonSimpleTransformation->setChecked(true);
            slot_setAdvancedFromSimpleTransformation();
        }
    }
    emit sigConfigurationUpdated();
}

KisPropertiesConfigurationSP KisScreentoneConfigWidget::configuration() const
{
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get(KisScreentoneGeneratorConfiguration::defaultName());
    KisScreentoneGeneratorConfigurationSP config =
        dynamic_cast<KisScreentoneGeneratorConfiguration*>(
            generator->factoryConfiguration(KisGlobalResourcesInterface::instance()).data()
        );
        
    config->setPattern(m_ui.comboBoxPattern->currentIndex());
    config->setShape(comboIndexToShape(m_ui.comboBoxPattern->currentIndex(), m_ui.comboBoxShape->currentIndex()));
    config->setInterpolation(m_ui.comboBoxInterpolation->currentIndex());
    
    config->setForegroundColor(m_ui.buttonForegroundColor->color());
    config->setForegroundOpacity(m_ui.sliderForegroundOpacity->value());
    config->setBackgroundColor(m_ui.buttonBackgroundColor->color());
    config->setBackgroundOpacity(m_ui.sliderBackgroundOpacity->value());
    config->setInvert(m_ui.checkBoxInvert->isChecked());
    config->setBrightness(m_ui.sliderBrightness->value());
    config->setContrast(m_ui.sliderContrast->value());

    config->setTransformationMode(m_ui.buttonAdvancedTransformation->isChecked() ? KisScreentoneTransformationMode_Advanced
                                                                                 : KisScreentoneTransformationMode_Simple);
    config->setUnits(m_ui.comboBoxUnits->currentIndex());
    config->setResolution(m_ui.sliderResolution->value());
    config->setFrequencyX(m_ui.sliderFrequencyX->value());
    config->setFrequencyY(m_ui.sliderFrequencyY->value());
    config->setConstrainFrequency(m_ui.buttonConstrainFrequency->keepAspectRatio());
    config->setPositionX(m_ui.sliderPositionX->value());
    config->setPositionY(m_ui.sliderPositionY->value());
    config->setSizeX(m_ui.sliderSizeX->value());
    config->setSizeY(m_ui.sliderSizeY->value());
    config->setConstrainSize(m_ui.buttonConstrainSize->keepAspectRatio());
    config->setShearX(m_ui.sliderShearX->value());
    config->setShearY(m_ui.sliderShearY->value());
    config->setRotation(m_ui.angleSelectorRotation->angle());

    return config;
}

void KisScreentoneConfigWidget::setView(KisViewManager *view)
{
    m_view = view;
    m_ui.buttonResolutionFromImage->setEnabled(true);
}

void KisScreentoneConfigWidget::setupPatternComboBox()
{
    m_ui.comboBoxPattern->clear();
    m_ui.comboBoxPattern->addItems(screentonePatternNames());
}

void KisScreentoneConfigWidget::setupShapeComboBox()
{
    m_ui.comboBoxShape->clear();
    QStringList names = screentoneShapeNames(m_ui.comboBoxPattern->currentIndex());
    if (names.isEmpty()) {
        m_ui.labelShape->hide();
        m_ui.comboBoxShape->hide();
    } else {
        m_ui.comboBoxShape->addItems(names);
        m_ui.labelShape->show();
        m_ui.comboBoxShape->show();
    }
}

void KisScreentoneConfigWidget::setupInterpolationComboBox()
{
    m_ui.comboBoxInterpolation->clear();
    QStringList names =
        screentoneInterpolationNames(
            m_ui.comboBoxPattern->currentIndex(),
            comboIndexToShape(m_ui.comboBoxPattern->currentIndex(), m_ui.comboBoxShape->currentIndex())
        );
    if (names.isEmpty()) {
        m_ui.labelInterpolation->hide();
        m_ui.comboBoxInterpolation->hide();
    } else {
        m_ui.comboBoxInterpolation->addItems(names);
        m_ui.labelInterpolation->show();
        m_ui.comboBoxInterpolation->show();
    }
}

int KisScreentoneConfigWidget::shapeToComboIndex(int pattern, int shape) const
{
    if (pattern == KisScreentonePatternType_Lines) {
        return shape;
    }
    if (shape == KisScreentoneShapeType_RoundDots) {
        return 0;
    } else if (shape == KisScreentoneShapeType_EllipseDotsLegacy) {
        return 1;
    } else if (shape == KisScreentoneShapeType_EllipseDots) {
        return 2;
    } else if (shape == KisScreentoneShapeType_DiamondDots) {
        return 3;
    } else if (shape == KisScreentoneShapeType_SquareDots) {
        return 4;
    }
    return -1;
}

int KisScreentoneConfigWidget::comboIndexToShape(int patternIndex, int shapeIndex) const
{
    if (patternIndex == KisScreentonePatternType_Lines) {
        return shapeIndex;
    }
    switch (shapeIndex) {
        case 0: return KisScreentoneShapeType_RoundDots;
        case 1: return KisScreentoneShapeType_EllipseDotsLegacy;
        case 2: return KisScreentoneShapeType_EllipseDots;
        case 3: return KisScreentoneShapeType_DiamondDots;
        case 4: return KisScreentoneShapeType_SquareDots;
    }
    return -1;
}

void KisScreentoneConfigWidget::slot_comboBoxPattern_currentIndexChanged(int)
{
    KisSignalsBlocker blocker(m_ui.comboBoxShape, m_ui.comboBoxInterpolation);
    setupShapeComboBox();
    setupInterpolationComboBox();
    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_comboBoxShape_currentIndexChanged(int)
{
    KisSignalsBlocker blocker(m_ui.comboBoxInterpolation);
    setupInterpolationComboBox();
    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_buttonSimpleTransformation_toggled(bool checked)
{
    if (!checked) {
        return;
    }

    m_ui.tabTransformation->setUpdatesEnabled(false);

    m_ui.containerPosition->hide();
    m_ui.containerSize->hide();
    m_ui.containerShear->hide();
    m_ui.labelPosition->hide();
    m_ui.labelSize->hide();
    m_ui.labelShear->hide();
    m_ui.layoutTransformation->takeRow(m_ui.containerPosition);
    m_ui.layoutTransformation->takeRow(m_ui.containerSize);
    m_ui.layoutTransformation->takeRow(m_ui.containerShear);

    // Prevent adding the widgets if thy are already in the layout
    if (!m_ui.containerResolution->isVisible()) {
        m_ui.layoutTransformation->insertRow(1, m_ui.labelResolution, m_ui.containerResolution);
        m_ui.layoutTransformation->insertRow(2, m_ui.labelFrequency, m_ui.containerFrequency);
        m_ui.containerResolution->show();
        m_ui.containerFrequency->show();
        m_ui.labelResolution->show();
        m_ui.labelFrequency->show();
    }

    m_ui.tabTransformation->setUpdatesEnabled(true);

    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_buttonAdvancedTransformation_toggled(bool checked)
{
    if (!checked) {
        return;
    }

    m_ui.tabTransformation->setUpdatesEnabled(false);

    m_ui.containerResolution->hide();
    m_ui.containerFrequency->hide();
    m_ui.labelResolution->hide();
    m_ui.labelFrequency->hide();
    m_ui.layoutTransformation->takeRow(m_ui.containerResolution);
    m_ui.layoutTransformation->takeRow(m_ui.containerFrequency);

    // Prevent adding the widgets if thy are already in the layout
    if (!m_ui.containerPosition->isVisible()) {
        m_ui.layoutTransformation->insertRow(1, m_ui.labelPosition, m_ui.containerPosition);
        m_ui.layoutTransformation->insertRow(2, m_ui.labelSize, m_ui.containerSize);
        m_ui.layoutTransformation->insertRow(3, m_ui.labelShear, m_ui.containerShear);
        m_ui.containerPosition->show();
        m_ui.containerSize->show();
        m_ui.containerShear->show();
        m_ui.labelPosition->show();
        m_ui.labelSize->show();
        m_ui.labelShear->show();
    }

    m_ui.tabTransformation->setUpdatesEnabled(true);

    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_comboBoxUnits_currentIndexChanged(int index)
{
    const QString resSuffix = index == 0 ? pixelsInchSuffix : pixelsCentimeterSuffix;
    const QString freqSuffix = index == 0 ? linesInchSuffix : linesCentimeterSuffix;
    m_ui.sliderResolution->setSuffix(resSuffix);
    m_ui.sliderFrequencyX->setSuffix(freqSuffix);
    m_ui.sliderFrequencyY->setSuffix(freqSuffix);
    {
        KisSignalsBlocker blocker(m_ui.sliderResolution, m_ui.sliderFrequencyX, m_ui.sliderFrequencyY);
        const KoUnit unitFrom = index == 1 ? KoUnit(KoUnit::Centimeter) : KoUnit(KoUnit::Inch);
        const KoUnit unitTo = index == 1 ? KoUnit(KoUnit::Inch) : KoUnit(KoUnit::Centimeter);
        m_ui.sliderResolution->setValue(
            KoUnit::convertFromUnitToUnit(
                m_ui.sliderResolution->value(), unitFrom, unitTo)
        );
        m_ui.sliderFrequencyX->setValue(
            KoUnit::convertFromUnitToUnit(m_ui.sliderFrequencyX->value(), unitFrom, unitTo)
        );
        m_ui.sliderFrequencyY->setValue(
            KoUnit::convertFromUnitToUnit(m_ui.sliderFrequencyY->value(), unitFrom, unitTo)
        );
    }
    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_buttonResolutionFromImage_clicked()
{
    if (m_view) {
        if (m_ui.comboBoxUnits->currentIndex() == 1) {
            m_ui.sliderResolution->setValue(
                KoUnit::convertFromUnitToUnit(m_view->image()->yRes(), KoUnit(KoUnit::Centimeter), KoUnit(KoUnit::Point))
            );
        } else {
            m_ui.sliderResolution->setValue(
                KoUnit::convertFromUnitToUnit(m_view->image()->yRes(), KoUnit(KoUnit::Inch), KoUnit(KoUnit::Point))
            );
        }
    }
}

void KisScreentoneConfigWidget::slot_sliderFrequencyX_valueChanged(qreal value)
{
    slot_setAdvancedFromSimpleTransformation();
    if (m_ui.buttonConstrainFrequency->keepAspectRatio()) {
        KisSignalsBlocker blocker(m_ui.sliderFrequencyY);
        m_ui.sliderFrequencyY->setValue(value);
    }
    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_sliderFrequencyY_valueChanged(qreal value)
{
    slot_setAdvancedFromSimpleTransformation();
    if (m_ui.buttonConstrainFrequency->keepAspectRatio()) {
        KisSignalsBlocker blocker(m_ui.sliderFrequencyX);
        m_ui.sliderFrequencyX->setValue(value);
    }
    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_buttonConstrainFrequency_keepAspectRatioChanged(bool keep)
{
    slot_setAdvancedFromSimpleTransformation();
    if (keep) {
        slot_sliderFrequencyX_valueChanged(m_ui.sliderFrequencyX->value());
    }
}

void KisScreentoneConfigWidget::slot_sliderSizeX_valueChanged(qreal value)
{
    slot_setSimpleFromAdvancedTransformation();
    if (m_ui.buttonConstrainSize->keepAspectRatio()) {
        KisSignalsBlocker blocker(m_ui.sliderSizeY);
        m_ui.sliderSizeY->setValue(value);
    }
    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_sliderSizeY_valueChanged(qreal value)
{
    slot_setSimpleFromAdvancedTransformation();
    if (m_ui.buttonConstrainSize->keepAspectRatio()) {
        KisSignalsBlocker blocker(m_ui.sliderSizeX);
        m_ui.sliderSizeX->setValue(value);
    }
    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_buttonConstrainSize_keepAspectRatioChanged(bool keep)
{
    slot_setSimpleFromAdvancedTransformation();
    if (keep) {
        slot_sliderSizeX_valueChanged(m_ui.sliderSizeX->value());
    }
}

void KisScreentoneConfigWidget::slot_setAdvancedFromSimpleTransformation()
{
    KisSignalsBlocker blocker1(m_ui.sliderPositionX, m_ui.sliderPositionY);
    KisSignalsBlocker blocker2(m_ui.sliderSizeX, m_ui.sliderSizeY, m_ui.buttonConstrainSize);
    KisSignalsBlocker blocker3(m_ui.sliderShearX, m_ui.sliderShearY);
    m_ui.sliderPositionX->setValue(0.0);
    m_ui.sliderPositionY->setValue(0.0);
    m_ui.sliderSizeX->setValue(m_ui.sliderResolution->value() / m_ui.sliderFrequencyX->value());
    m_ui.sliderSizeY->setValue(m_ui.sliderResolution->value() / m_ui.sliderFrequencyY->value());
    m_ui.buttonConstrainSize->setKeepAspectRatio(m_ui.buttonConstrainFrequency->keepAspectRatio());
    m_ui.sliderShearX->setValue(0.0);
    m_ui.sliderShearY->setValue(0.0);
}

void KisScreentoneConfigWidget::slot_setSimpleFromAdvancedTransformation()
{
    KisSignalsBlocker blocker(m_ui.sliderFrequencyX, m_ui.sliderFrequencyY, m_ui.buttonConstrainFrequency);
    m_ui.sliderFrequencyX->setValue(m_ui.sliderResolution->value() / m_ui.sliderSizeX->value());
    m_ui.sliderFrequencyY->setValue(m_ui.sliderResolution->value() / m_ui.sliderSizeY->value());
    m_ui.buttonConstrainFrequency->setKeepAspectRatio(m_ui.buttonConstrainSize->keepAspectRatio());
}
