/*
 * Copyright (c) 2013-2016 Thomas Isaac Lightburn
 *
 *
 * This file is part of OpenKJ.
 *
 * OpenKJ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlgsettings.h"
#include "ui_dlgsettings.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QFontDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QAudioRecorder>
#include <QSqlQuery>
#include <QtSql>
#include <QXmlStreamWriter>
#include <QNetworkReply>
#include <QAuthenticator>
#include "audiorecorder.h"


extern Settings *settings;
extern OKJSongbookAPI *songbookApi;


DlgSettings::DlgSettings(AbstractAudioBackend *AudioBackend, AbstractAudioBackend *BmAudioBackend, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgSettings)
{
    pageSetupDone = false;
    kAudioBackend = AudioBackend;
    bmAudioBackend = BmAudioBackend;
    networkManager = new QNetworkAccessManager(this);
    ui->setupUi(this);
    ui->spinBoxHAdjust->setValue(settings->cdgHSizeAdjustment());
    ui->spinBoxVAdjust->setValue(settings->cdgVSizeAdjustment());
    ui->spinBoxHOffset->setValue(settings->cdgHOffset());
    ui->spinBoxVOffset->setValue(settings->cdgVOffset());
    createIcons();
    ui->listWidget->setCurrentRow(0);
    QStringList screens = getMonitors();
    ui->listWidgetMonitors->addItems(screens);
    audioOutputDevices = kAudioBackend->getOutputDevices();
    ui->listWidgetAudioDevices->addItems(audioOutputDevices);
    int selDevice = audioOutputDevices.indexOf(settings->audioOutputDevice());
    if (selDevice == -1)
        ui->listWidgetAudioDevices->item(0)->setSelected(true);
    else
    {
        ui->listWidgetAudioDevices->item(selDevice)->setSelected(true);
        kAudioBackend->setOutputDevice(selDevice);
    }
    ui->listWidgetAudioDevicesBm->addItems(audioOutputDevices);
    selDevice = audioOutputDevices.indexOf(settings->audioOutputDeviceBm());
    if (selDevice == -1)
        ui->listWidgetAudioDevicesBm->item(0)->setSelected(true);
    else
    {
        ui->listWidgetAudioDevicesBm->item(selDevice)->setSelected(true);
        bmAudioBackend->setOutputDevice(selDevice);
    }
    ui->checkBoxShowCdgWindow->setChecked(settings->showCdgWindow());
    ui->groupBoxMonitors->setChecked(settings->cdgWindowFullscreen());
    ui->listWidgetMonitors->setEnabled(settings->showCdgWindow());
    ui->groupBoxMonitors->setEnabled(settings->showCdgWindow());
    if (screens.count() > settings->cdgWindowFullScreenMonitor())
        ui->listWidgetMonitors->item(settings->cdgWindowFullScreenMonitor())->setSelected(true);
    else
        settings->setCdgWindowFullscreen(false);
    ui->spinBoxTickerHeight->setValue(settings->tickerHeight());
    ui->horizontalSliderTickerSpeed->setValue(settings->tickerSpeed());
    QPalette txtpalette = ui->pushButtonTextColor->palette();
    txtpalette.setColor(QPalette::Button, settings->tickerTextColor());
    ui->pushButtonTextColor->setPalette(txtpalette);
    QPalette bgpalette = ui->pushButtonBgColor->palette();
    bgpalette.setColor(QPalette::Button, settings->tickerBgColor());
    ui->pushButtonBgColor->setPalette(bgpalette);
    if (settings->tickerFullRotation())
    {
        ui->radioButtonFullRotation->setChecked(true);
        ui->spinBoxTickerSingers->setEnabled(false);
    }
    else
    {
        ui->radioButtonPartialRotation->setChecked(true);
        ui->spinBoxTickerSingers->setEnabled(true);
    }
    ui->spinBoxTickerSingers->setValue(settings->tickerShowNumSingers());
    ui->groupBoxRequestServer->setChecked(settings->requestServerEnabled());
    ui->lineEditUrl->setText(settings->requestServerUrl());
    ui->lineEditApiKey->setText(settings->requestServerApiKey());
    ui->checkBoxIgnoreCertErrors->setChecked(settings->requestServerIgnoreCertErrors());
    if ((settings->bgMode() == settings->BG_MODE_IMAGE) || (settings->bgSlideShowDir() == ""))
        ui->rbBgImage->setChecked(true);
    else
        ui->rbSlideshow->setChecked(true);
    ui->lineEditCdgBackground->setText(settings->cdgDisplayBackgroundImage());
    ui->lineEditSlideshowDir->setText(settings->bgSlideShowDir());
    ui->checkBoxFader->setChecked(settings->audioUseFader());
    ui->checkBoxDownmix->setChecked(settings->audioDownmix());
    ui->checkBoxSilenceDetection->setChecked(settings->audioDetectSilence());
    ui->checkBoxFaderBm->setChecked(settings->audioUseFaderBm());
    ui->checkBoxDownmixBm->setChecked(settings->audioDownmixBm());
    ui->checkBoxSilenceDetectionBm->setChecked(settings->audioDetectSilenceBm());
    AudioRecorder recorder;
    QAudioRecorder audioRecorder;
    QStringList inputs = recorder.getDeviceList();
    QStringList codecs = recorder.getCodecs();
//    QStringList containers = audioRecorder.supportedContainers();
    ui->groupBoxRecording->setChecked(settings->recordingEnabled());
    ui->comboBoxDevice->addItems(inputs);
    ui->comboBoxCodec->addItems(codecs);
//    ui->comboBoxContainer->addItems(containers);
    QString recordingInput = settings->recordingInput();
    if (recordingInput == "undefined")
        ui->comboBoxDevice->setCurrentIndex(0);
    else
        ui->comboBoxDevice->setCurrentIndex(ui->comboBoxDevice->findText(settings->recordingInput()));
    QString recordingCodec = settings->recordingCodec();
    if (recordingCodec == "undefined")
        ui->comboBoxCodec->setCurrentIndex(1);
    else
        ui->comboBoxCodec->setCurrentIndex(ui->comboBoxCodec->findText(settings->recordingCodec()));
    ui->lineEditOutputDir->setText(settings->recordingOutputDir());
    ui->lineEditTickerMessage->setText(settings->tickerCustomString());
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onNetworkReply(QNetworkReply*)));
    connect(networkManager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(onSslErrors(QNetworkReply*)));
    connect(settings, SIGNAL(tickerHeightChanged(int)), ui->spinBoxTickerHeight, SLOT(setValue(int)));
    connect(ui->spinBoxHAdjust, SIGNAL(valueChanged(int)), settings, SLOT(setCdgHSizeAdjustment(int)));
    connect(ui->spinBoxVAdjust, SIGNAL(valueChanged(int)), settings, SLOT(setCdgVSizeAdjustment(int)));
    connect(ui->spinBoxHOffset, SIGNAL(valueChanged(int)), settings, SLOT(setCdgHOffset(int)));
    connect(ui->spinBoxVOffset, SIGNAL(valueChanged(int)), settings, SLOT(setCdgVOffset(int)));
    pageSetupDone = true;
}

DlgSettings::~DlgSettings()
{
    delete ui;
}

QStringList DlgSettings::getMonitors()
{
    QStringList screenStrings;
    for (int i=0; i < QApplication::desktop()->screenCount(); i++)
    {
        int sWidth = QApplication::desktop()->screenGeometry(i).width();
        int sHeight = QApplication::desktop()->screenGeometry(i).height();
        screenStrings << "Monitor " + QString::number(i) + " - " + QString::number(sWidth) + "x" + QString::number(sHeight);
    }
    return screenStrings;
}

void DlgSettings::onNetworkReply(QNetworkReply *reply)
{
    Q_UNUSED(reply);
}

void DlgSettings::onSslErrors(QNetworkReply *reply)
{
    if (settings->requestServerIgnoreCertErrors())
        reply->ignoreSslErrors();
    else
    {
        QMessageBox::warning(this, "SSL Error", "Unable to establish secure conneciton with server.");
    }
}

void DlgSettings::createIcons()
{
    QListWidgetItem *audioButton = new QListWidgetItem(ui->listWidget);
    audioButton->setIcon(QIcon(":/icons/Icons/audio-card.png"));
    audioButton->setText(tr("Audio"));
    audioButton->setTextAlignment(Qt::AlignHCenter);
    audioButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    QListWidgetItem *videoButton = new QListWidgetItem(ui->listWidget);
    videoButton->setIcon(QIcon(":/icons/Icons/video-display.png"));
    videoButton->setText(tr("Video"));
    videoButton->setTextAlignment(Qt::AlignHCenter);
    videoButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    QListWidgetItem *networkButton = new QListWidgetItem(ui->listWidget);
    networkButton->setIcon(QIcon(":/icons/Icons/network-wired.png"));
    networkButton->setText(tr("Network"));
    networkButton->setTextAlignment(Qt::AlignHCenter);
    networkButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void DlgSettings::on_btnClose_clicked()
{
    close();
}

void DlgSettings::on_checkBoxShowCdgWindow_stateChanged(int arg1)
{
    settings->setShowCdgWindow(arg1);
    ui->listWidgetMonitors->setEnabled(arg1);
    ui->groupBoxMonitors->setEnabled(arg1);
    ui->pushButtonBrowse->setEnabled(arg1);
    ui->lineEditCdgBackground->setEnabled(arg1);
    ui->pushButtonClearBgImg->setEnabled(arg1);
}

void DlgSettings::on_groupBoxMonitors_toggled(bool arg1)
{
    settings->setCdgWindowFullscreen(arg1);
    //emit cdgWindowFullScreenChanged(arg1);
}

void DlgSettings::on_listWidgetMonitors_itemSelectionChanged()
{
    int selMonitor = ui->listWidgetMonitors->selectionModel()->selectedIndexes().at(0).row();
    settings->setCdgWindowFullscreenMonitor(selMonitor);
}

void DlgSettings::on_pushButtonFont_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, settings->tickerFont(), this, "Select ticker font");
    if (ok)
    {
        settings->setTickerFont(font);
    }
}

void DlgSettings::on_spinBoxTickerHeight_valueChanged(int arg1)
{
    settings->setTickerHeight(arg1);
}

void DlgSettings::on_horizontalSliderTickerSpeed_valueChanged(int value)
{
    settings->setTickerSpeed(value);
}

void DlgSettings::on_pushButtonTextColor_clicked()
{
    QColor color = QColorDialog::getColor(settings->tickerTextColor(),this,"Select ticker text color");
    if (color.isValid())
    {
        settings->setTickerTextColor(color);
        QPalette palette = ui->pushButtonTextColor->palette();
        palette.setColor(ui->pushButtonTextColor->backgroundRole(), color);
        ui->pushButtonTextColor->setPalette(palette);
    }
}

void DlgSettings::on_pushButtonBgColor_clicked()
{
    QColor color = QColorDialog::getColor(settings->tickerTextColor(),this,"Select ticker background color");
    if (color.isValid())
    {
        settings->setTickerBgColor(color);
        QPalette palette = ui->pushButtonBgColor->palette();
        palette.setColor(ui->pushButtonBgColor->backgroundRole(), color);
        ui->pushButtonBgColor->setPalette(palette);
    }
}

void DlgSettings::on_radioButtonFullRotation_toggled(bool checked)
{
    settings->setTickerFullRotation(checked);
    ui->spinBoxTickerSingers->setEnabled(!checked);
}



void DlgSettings::on_spinBoxTickerSingers_valueChanged(int arg1)
{
    settings->setTickerShowNumSingers(arg1);
}

void DlgSettings::on_groupBoxTicker_toggled(bool arg1)
{
    settings->setTickerEnabled(arg1);
}

void DlgSettings::on_lineEditUrl_editingFinished()
{
    settings->setRequestServerUrl(ui->lineEditUrl->text());
}

void DlgSettings::on_checkBoxIgnoreCertErrors_toggled(bool checked)
{
    settings->setRequestServerIgnoreCertErrors(checked);
}

void DlgSettings::on_groupBoxRequestServer_toggled(bool arg1)
{
    settings->setRequestServerEnabled(arg1);
}

void DlgSettings::on_pushButtonBrowse_clicked()
{
    QString imageFile = QFileDialog::getOpenFileName(this,QString("Select image file"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation), QString("Images (*.png *.jpg *.jpeg *.gif)"));
    if (imageFile != "")
    {
        QImage image(imageFile);
        if (!image.isNull())
        {
            settings->setCdgDisplayBackgroundImage(imageFile);
            ui->lineEditCdgBackground->setText(imageFile);
        }
        else
        {
            QMessageBox::warning(this, tr("Image load error"),QString("Unsupported or corrupt image file."));
        }
    }
}

void DlgSettings::on_checkBoxFader_toggled(bool checked)
{
    settings->setAudioUseFader(checked);
    emit audioUseFaderChanged(checked);
}

void DlgSettings::on_checkBoxFaderBm_toggled(bool checked)
{
    settings->setAudioUseFaderBm(checked);
    emit audioUseFaderChangedBm(checked);
}

void DlgSettings::on_checkBoxSilenceDetection_toggled(bool checked)
{
    settings->setAudioDetectSilence(checked);
    emit audioSilenceDetectChanged(checked);
}

void DlgSettings::on_checkBoxSilenceDetectionBm_toggled(bool checked)
{
    settings->setAudioDetectSilenceBm(checked);
    emit audioSilenceDetectChangedBm(checked);
}

void DlgSettings::on_checkBoxDownmix_toggled(bool checked)
{
    settings->setAudioDownmix(checked);
    emit audioDownmixChanged(checked);
}

void DlgSettings::on_checkBoxDownmixBm_toggled(bool checked)
{
    settings->setAudioDownmixBm(checked);
    emit audioDownmixChangedBm(checked);
}

void DlgSettings::on_listWidgetAudioDevices_itemSelectionChanged()
{
    if (pageSetupDone)
    {
        QString device = ui->listWidgetAudioDevices->selectedItems().at(0)->text();
        settings->setAudioOutputDevice(device);
        int deviceIndex = audioOutputDevices.indexOf(QRegExp(device,Qt::CaseSensitive,QRegExp::FixedString));
        if (deviceIndex != -1)
            kAudioBackend->setOutputDevice(deviceIndex);
    }
}

void DlgSettings::on_listWidgetAudioDevicesBm_itemSelectionChanged()
{
    if (pageSetupDone)
    {
        QString device = ui->listWidgetAudioDevicesBm->selectedItems().at(0)->text();
        settings->setAudioOutputDeviceBm(device);
        int deviceIndex = audioOutputDevices.indexOf(QRegExp(device,Qt::CaseSensitive,QRegExp::FixedString));
        if (deviceIndex != -1)
            bmAudioBackend->setOutputDevice(deviceIndex);
    }
}

//void DlgSettings::on_comboBoxBackend_currentIndexChanged(int index)
//{
////    if (pageSetupDone)
////    {
////        qDebug() << "SettingsDialog::on_comboBoxBackend_currentIndexChanged(" << index << ") fired";
////        if (kAudioBackend->state() != AbstractAudioBackend::StoppedState)
////        {
////            QMessageBox::warning(this, "Unable to complete action","You can not change the audio backend while there is an active song playing or paused.  Please stop the current song and try again");
////            ui->comboBoxBackend->setCurrentIndex(settings->audioBackend());
////        }
////        else
////            settings->setAudioBackend(index);
////    }
//}

//void DlgSettings::audioBackendChanged(int index)
//{
////    pageSetupDone = false;
////    kAudioBackend = audioBackends->at(index);
////    ui->checkBoxSilenceDetection->setHidden(!kAudioBackend->canDetectSilence());
////    ui->checkBoxDownmix->setHidden(!kAudioBackend->canDownmix());
////    ui->checkBoxFader->setHidden(!kAudioBackend->canFade());
////    ui->listWidgetAudioDevices->clear();
////    ui->listWidgetAudioDevices->addItems(kAudioBackend->getOutputDevices());
////    pageSetupDone = true;
////    if (kAudioBackend->getOutputDevices().contains(settings->audioOutputDevice()))
////    {
////        ui->listWidgetAudioDevices->findItems(settings->audioOutputDevice(),Qt::MatchExactly).at(0)->setSelected(true);
////    }
////    else
//////        ui->listWidgetAudioDevices->item(0)->setSelected(true);
////    if (!kAudioBackend->downmixChangeRequiresRestart())
////        ui->checkBoxDownmix->setText("Downmix to mono");
////    else
////        ui->checkBoxDownmix->setText("Dowmix to mono (requires restart)");
//}

void DlgSettings::on_comboBoxDevice_currentIndexChanged(const QString &arg1)
{
    if (pageSetupDone)
        settings->setRecordingInput(arg1);
}

void DlgSettings::on_comboBoxCodec_currentIndexChanged(const QString &arg1)
{
    if (pageSetupDone)
    {
        settings->setRecordingCodec(arg1);
        if (arg1 == "audio/mpeg")
        {
            settings->setRecordingRawExtension("mp3");
        }
    }
}

void DlgSettings::on_groupBoxRecording_toggled(bool arg1)
{
    if (pageSetupDone)
        settings->setRecordingEnabled(arg1);
}

void DlgSettings::on_buttonBrowse_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Select the output directory",QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
    if (dirName != "")
    {
        settings->setRecordingOutputDir(dirName);
        ui->lineEditOutputDir->setText(dirName);
    }
}

void DlgSettings::on_pushButtonClearBgImg_clicked()
{
    settings->setCdgDisplayBackgroundImage(QString::null);
    ui->lineEditCdgBackground->setText(QString::null);
}

void DlgSettings::on_pushButtonSlideshowBrowse_clicked()
{
    QString initialPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (settings->bgSlideShowDir() != "")
        initialPath = settings->bgSlideShowDir();
    QString dirName = QFileDialog::getExistingDirectory(this, "Select the slideshow directory", initialPath);
    if (dirName != "")
    {
        settings->setBgSlideShowDir(dirName);
        ui->lineEditSlideshowDir->setText(dirName);
    }
}

void DlgSettings::on_rbSlideshow_toggled(bool checked)
{
    if (checked)
        settings->setBgMode(settings->BG_MODE_SLIDESHOW);
    else
        settings->setBgMode(settings->BG_MODE_IMAGE);
}

void DlgSettings::on_rbBgImage_toggled(bool checked)
{
    if (checked)
        settings->setBgMode(settings->BG_MODE_IMAGE);
    else
        settings->setBgMode(settings->BG_MODE_SLIDESHOW);
}

void DlgSettings::on_lineEditApiKey_editingFinished()
{
    settings->setRequestServerApiKey(ui->lineEditApiKey->text());
}

void DlgSettings::on_lineEditTickerMessage_textChanged(const QString &arg1)
{
    settings->setTickerCustomString(arg1);
}
