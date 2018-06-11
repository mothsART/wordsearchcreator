#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include "wordsearchapplication.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    QSettings settings;
    bool autoCheckEnabled = settings.value("updateChecker/autoUpdateCheckEnabled", true).toBool();
    ui->updatesCheckBox->setChecked(autoCheckEnabled);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::on_buttonBox_accepted()
{
    QSettings settings;
    settings.setValue("updateChecker/autoUpdateCheckEnabled",  ui->updatesCheckBox->checkState());
}
