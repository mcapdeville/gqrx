/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           http://gqrx.dk/
 *
 * Copyright 2011-2013 Alexandru Csete OZ9AEC.
 *
 * Gqrx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gqrx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gqrx; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#include <QDebug>
#include <QVariant>
#include "dockrtty.h"
#include "ui_dockrtty.h"

DockRTTY::DockRTTY(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DockRTTY)
{
    ui->setupUi(this);
    ui->mode->addItem(QString("5 bits baudot"));
    ui->mode->addItem(QString("7 bits ascii"));
    ui->mode->addItem(QString("8 bits ascii"));

    ui->parity->addItem(QString("none"));
    ui->parity->addItem(QString("odd"));
    ui->parity->addItem(QString("even"));
    ui->parity->addItem(QString("mark"));
    ui->parity->addItem(QString("space"));
    ui->parity->addItem(QString("dont care"));
}

DockRTTY::~DockRTTY()
{
    delete ui;
}

void DockRTTY::update_text(QString text)
{
	ui->text->moveCursor(QTextCursor::End);
        ui->text->insertPlainText(text);
}

void DockRTTY::ClearText()
{
    ui->text->setText("");
}

void DockRTTY::show_Enabled()
{
    if (!ui->rttyCheckBox->isChecked())
    {
        ui->rttyCheckBox->blockSignals(true);
        ui->rttyCheckBox->setChecked(true);
        ui->rttyCheckBox->blockSignals(false);
    }
}

void DockRTTY::show_Disabled()
{
}

void DockRTTY::set_Disabled()
{
    ui->rttyCheckBox->setDisabled(true);
    ui->rttyCheckBox->blockSignals(true);
    ui->rttyCheckBox->setChecked(false);
    ui->rttyCheckBox->blockSignals(false);
    ui->baud_rate->setDisabled(true);
    ui->mark_freq->setDisabled(true);
    ui->space_freq->setDisabled(true);
    ui->text->setDisabled(true);
    ui->mode->setDisabled(true);
}

void DockRTTY::set_Enabled()
{
    ui->rttyCheckBox->setDisabled(false);
    ui->baud_rate->setDisabled(false);
    ui->mark_freq->setDisabled(false);
    ui->space_freq->setDisabled(false);
    ui->text->setDisabled(false);
    ui->mode->setDisabled(false);
}

void DockRTTY::set_baud_rate(float baud_rate) {
	ui->baud_rate->setValue(baud_rate);
}

void DockRTTY::set_mark_freq(float mark_freq) {
	ui->mark_freq->setValue(mark_freq);
}

void DockRTTY::set_space_freq(float space_freq) {
	ui->space_freq->setValue(space_freq);
}

/** Enable/disable RTTY decoder */
void DockRTTY::on_rttyCheckBox_toggled(bool checked)
{
	if (checked)
    		emit rtty_start_decoder();
	else
		emit rtty_stop_decoder();
}

void DockRTTY::on_baud_rate_editingFinished() {
    emit rtty_baud_rate_Changed(ui->baud_rate->value());
}

void DockRTTY::on_mark_freq_editingFinished() {
    emit rtty_mark_freq_Changed(ui->mark_freq->value());
}

void DockRTTY::on_space_freq_editingFinished() {
    emit rtty_space_freq_Changed(ui->space_freq->value());
}

void DockRTTY::on_mode_currentIndexChanged(int mode) {
    emit rtty_mode_Changed(mode);
}

void DockRTTY::on_parity_currentIndexChanged(int parity) {
    emit rtty_parity_Changed(parity);
}
