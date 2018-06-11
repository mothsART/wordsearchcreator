/***************************************************************************
 *   Copyright (C) 2006-2013 M Wellings                                    *
 *   info@openforeveryone.co.uk                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation                             *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "wordsearchcontrol.h"

#include "wordsearchapplication.h"
#include "wordsearchdoc.h"
#include "appearancedialog.h"
#include "listcreator.h"
#include "directionselectordialogue.h"
#include "maskdialogue.h"
#include "pagelayoutdialogue.h"
#include "alphabetdialogue.h"
#include "mainwindow.h"

WordSearchControl::WordSearchControl(QWidget *)
{
    setupUi(this);
    usewordsSpinBox->setVisible(false);
    http= new QHttp(this);
    connect(titlebox, SIGNAL(textChanged(QString)), this, SIGNAL(titleChanged(QString)));
    connect(arscheckbox, SIGNAL(stateChanged(int)), this, SIGNAL(arsstateChanged(int)));
    connect(sacheckbox, SIGNAL(stateChanged(int)), this, SIGNAL(sastateChanged(int)));
    connect(swcheckbox, SIGNAL(stateChanged(int)), this, SIGNAL(swstateChanged(int)));
    connect(xspin, SIGNAL(valueChanged(int)), this, SIGNAL(XValChanged(int)));
    connect(yspin, SIGNAL(valueChanged(int)), this, SIGNAL(YValChanged(int)));
    connect(createbutton, SIGNAL(clicked()), this, SLOT(createclicked()));
    connect(WordListBox, SIGNAL(textChanged()), this, SLOT(updateNoLines()));	connect(http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
}

void WordSearchControl::createclicked()
{
    createDialog *cd = new createDialog(this, wsd);
    wsd->ws->wordlist.clear();
    Word temp;
    QTextCursor newCursor(WordListBox->document());
    int i=0;
    do{
        if (newCursor.block().text().trimmed()!="")
        {
            temp.word = newCursor.block().text().trimmed();
            temp.listpos = i; i++;
            wsd->ws->wordlist.push_back(temp);
        }
    }
    while(newCursor.movePosition(QTextCursor::Down));
    cd->exec();
    xspin->setValue(wsd->ws->XSize());
    yspin->setValue(wsd->ws->YSize());
    if (wsd->numwordsplaced()!=wsd->ws->wordlist.size()) {
        QString str;
        str.setNum(wsd->numwordsplaced());
        if (wsd->numwordsplaced()==0)
            QMessageBox::information(this, tr("Word Search Creator"), tr("No words could be placed."));
        else if (wsd->numwordsplaced()==1)
            QMessageBox::information(this, tr("Word Search Creator"), tr("Only 1 word could be placed."));
        else
            QMessageBox::information(this, tr("Word Search Creator"), tr("Only ") + str + tr(" words could be placed."));
    }
    delete cd;
}

void WordSearchControl::updateNoLines()
{
    QTextCursor newCursor(WordListBox->document());
    int count=0;
    do{
        if (newCursor.block().text().trimmed()!="")
        {
            count++;
        }
    }
    while(newCursor.movePosition(QTextCursor::Down));
    QString str;
    str.setNum(count);
    wordcountLabel->setText("Words: " + str);
}

int WordSearchControl::SaveAs()
{
    QString fileName;
    QString path;
    if (wsd->filename.isEmpty())
        path = QDir::homePath();
    else
    {
        QFileInfo fi(wsd->filename);
        path = fi.absoluteDir().absolutePath();
    }
    while (true) {
        fileName = QFileDialog::getSaveFileName(this,
                                                tr("Save Word Search"),
                                                path,
                                                tr("Word search files (*.wordsearch)"),
                                                0, QFileDialog::DontConfirmOverwrite);
        if (fileName.isEmpty())
            return false;

        if ((fileName.mid(fileName.size()-4,4) != ".wsh") && (fileName.mid(fileName.size()-11,11) != ".wordsearch"))
            fileName = fileName + ".wordsearch";

        const QFileInfo fi(fileName);
        if (!fi.exists())
            break;

        const QString prompt = tr("%1 already exists.\nDo you want to replace it?").arg(fi.fileName());
        if (QMessageBox::warning(this, tr("Save Word Search"), prompt, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            break;

    }
    wsd->filename=fileName;
    return this->Save();
}

int WordSearchControl::Save()
{
    if (wsd->filename == "")
    {
        return SaveAs();
    }
    QFile file(wsd->filename);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    this->parentWidget()->parentWidget()->setWindowFilePath(wsd->filename);
    WordSearchApplication *app = static_cast<WordSearchApplication*>(QApplication::instance());
    app->updateWindowLists();
//    this->parentWidget()->parentWidget ()->setWindowTitle(wsd->filename + "[*] - Word Search Creator");
    wsd->saveToIO(file);
    wsd->setEditedState(false);
    file.close();
    //qDebug() << "File saved" << wsd->filename;
    return true;
}

void WordSearchControl::upload()
{
    if (!QMessageBox::question(
            this,
            tr("Word Search Creator"),
            tr("You are about to send this word search over the internet to wordsearchcreator.org unencrypted. Do you wish to continue?"),
            tr("&Yes"), tr("&No"), 0, 2))
    {
        QBuffer wsdatabuffer;
        wsdatabuffer.open(QIODevice::WriteOnly);
        wsd->saveToIO(wsdatabuffer);
        http->setHost("wordsearchcreator.org");
        webbuffer = new QBuffer;
        httpPostId = http->post("/uploadws_1.0.php", wsdatabuffer.buffer(), webbuffer);
    }
}

void WordSearchControl::httpRequestFinished(int requestId, bool error)
{
    if (requestId != httpPostId) return;

    if (http->lastResponse().statusCode() != 200) {
        QMessageBox::information(this, tr("HTTP"), tr("Upload failed: %1.") .arg(http->lastResponse().reasonPhrase()));
        delete webbuffer; return;
    }
    if (error) {
        QMessageBox::information(this, tr("HTTP"), tr("Upload failed: %1.") .arg(http->errorString()));
        delete webbuffer;
        return;

    } else {
        QUrl url("http://wordsearchcreator.org/uploadws_1.0_step2.php");
        url.addQueryItem("id",webbuffer->buffer());
        QDesktopServices::openUrl(url);
        delete webbuffer;
    }
}

void WordSearchControl::SetupTemplate()
{
    int error = wsd->Open(":/template/blank.wsh");
    if (error)
    {
        qDebug() << "Error Opening template" << error;
    }
    wsd->filename = "";
    UpdateConts();
    this->parentWidget()->parentWidget()->setWindowFilePath(QString("Untitled"));
    WordSearchApplication *app = static_cast<WordSearchApplication*>(QApplication::instance());
    app->updateWindowLists();
//    this->parentWidget()->parentWidget ()->setWindowTitle("Untitled[*] - Word Search Creator");
    wsd->setEditedState(false);
}

void WordSearchControl::New()
{
    MainWindow *window = new MainWindow();
    window->show();
}

void WordSearchControl::appearance()
{
    appearanceDialog *apd = new appearanceDialog(this);
    apd->setDoc(wsd);
    apd->exec();
}

void WordSearchControl::showdirectionselector()
{
    DirectionSelectorDialog *ds = new DirectionSelectorDialog(wsd->ws, this);
    ds->exec();
}

void WordSearchControl::showmaskdrawer()
{
    MaskDialog *md = new MaskDialog(wsd, this);
    md->exec();
}

void WordSearchControl::Open(QString fileName)
{
    QString path;
    if (wsd->filename.isEmpty())
        path = QDir::homePath();
    else
    {
        QFileInfo fi(wsd->filename);
        path = fi.absoluteDir().absolutePath();
    }
    if (fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this,
                                                tr("Open Word Search"),
                                                path,
                                                tr("Wordsearch files (*.wordsearch *.wsh);;All Files (*)"));
    WordSearchDoc *newwsd = new WordSearchDoc;
    int error = newwsd->Open(fileName);
    if (error)
    {
        //qDebug() << "Error" << error;
        delete newwsd;
        return;
    }
    MainWindow *window = new MainWindow(newwsd);
    window->show();
    newwsd->setEditedState(false);
    if (wsd->filename == "" && wsd->getEdited()==false)
        this->parentWidget()->parentWidget()->close();
}

void WordSearchControl::UpdateConts()
{
    sacheckbox->setChecked(wsd->getShowAnswers());
    swcheckbox->setChecked(wsd->getShowWords());
    titlebox->setText(wsd->getTitle());
    WordListBox->document()->clear();
    QTextCursor newCursor(WordListBox->document());
    QList<Word> sortedwordlist = wsd->sortedWordList();
    for (int a=0; a < sortedwordlist.size(); a++)
    {
        newCursor.insertText(sortedwordlist.at(a).word);
        newCursor.insertBlock();
    }
    xspin->setValue(wsd->ws->XSize());
    yspin->setValue(wsd->ws->YSize());
    titlebox->setText(wsd->getTitle());
    this->parentWidget()->parentWidget()->setWindowFilePath(wsd->filename);
    WordSearchApplication *app = static_cast<WordSearchApplication*>(QApplication::instance());
    app->updateWindowLists();
//    this->parentWidget()->parentWidget()->setWindowTitle(wsd->filename + "[*] - Word Search Creator");
    useMaskChanged(wsd->getUseMask());
}

void WordSearchControl::showlistCreator()
{
    listCreator *wlc = new listCreator(this);
    if (wlc->exec())
    {
        QStringList wl = wlc->wl;
        for (int a = 0; a < wl.count(); a++)
        {
            WordListBox->insertPlainText(wl.at(a)+"\n");
        }
        delete wlc;
    }
}

void WordSearchControl::showpageLayout()
{
    PageLayoutDialog *pld = new PageLayoutDialog(this);
    pld->setPageWidth(wsd->pagewidth);
    pld->setPageHeight(wsd->pageheight);
    pld->setMarginTop(wsd->topMargin);
    pld->setMarginLeft(wsd->leftMargin);
    pld->setMarginRight(wsd->rightMargin);
    pld->setMarginBottom(wsd->bottomMargin);
    if (pld->exec())
    {
        wsd->pagewidth = pld->PageWidth();
        wsd->pageheight = pld->PageHeight();
        wsd->topMargin = pld->MarginTop();
        wsd->leftMargin = pld->MarginLeft();
        wsd->rightMargin = pld->MarginRight();
        wsd->bottomMargin = pld->MarginBottom();
        wsd->qpps = pld->qpps();
        wsd->setEditedState(true);
    }
    delete pld;
}

void WordSearchControl::setWordSearchDoc(WordSearchDoc *wsd)
{
    this->wsd=wsd;
    //connect(this, SIGNAL(Createclicked()), wsd, SLOT(Create()));
    connect(this, SIGNAL(Fontclicked()), wsd, SLOT(setWSFont()));
    connect(this, SIGNAL(XValChanged(int)), wsd, SLOT(setXSize(int)));
    connect(this, SIGNAL(YValChanged(int)), wsd, SLOT(setYSize(int)));
    connect(this, SIGNAL(titleChanged(QString)), wsd, SLOT(setTitle(QString)));
    connect(this, SIGNAL(arsstateChanged(int)), wsd, SLOT(setAutoResize(int)));
    connect(this, SIGNAL(swstateChanged(int)), wsd, SLOT(setShowWords(int)));
    connect(this, SIGNAL(sastateChanged(int)), wsd, SLOT(setShowAnswers(int)));
    connect(wsd, SIGNAL(useMaskChanged(int)), this, SLOT(useMaskChanged(int)));
    connect(wsd, SIGNAL(EditedStateChanged(bool)), this->parentWidget()->parentWidget(),SLOT(setWindowModified(bool)));
    UpdateConts();
}

createDialog::createDialog(QWidget *parent, WordSearchDoc *wsd)
    : QDialog(parent)
{
    this->wsd = wsd;
    this->setWindowModality(Qt::WindowModal);
    QLabel *label = new QLabel("Creating Wordsearch...");
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    QHBoxLayout *layout1 = new QHBoxLayout;
    QPushButton *stop = new QPushButton("Stop", this);
    stop->setIcon(QIcon(":/icons/stop.png"));
    layout1->addStretch();
    layout1->addWidget(stop);
    layout->addLayout(layout1);
    this->setLayout(layout);
    connect(stop, SIGNAL(clicked()), this, SLOT(reject()));
    connect(this, SIGNAL(rejected()), this, SLOT(stop()));
    QTimer::singleShot(0, this, SLOT(createws()));
}

void createDialog::createws()
{
    wsd->Create();
    this->accept();
}

void createDialog::stop()
{
    wsd->ws->stop=true;
}

void WordSearchControl::alphabet()
{
    AlphabetDialog *abd = new AlphabetDialog(this);
    abd->setDoc(wsd);
    abd->exec();
}

void WordSearchControl::setFooter()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Word Search Creator"),
                                         tr("Footer"), QLineEdit::Normal,
                                         wsd->getFooter(), &ok);
    if (ok)
        wsd->setFooter(text);
    if (text=="")
        wsd->setFooter(QString());
}


void WordSearchControl::useMaskChanged(int used)
{

    if (used)
    {
        xspin->setEnabled(false);
        yspin->setEnabled(false);
        arscheckbox->setEnabled(false);
        arscheckbox->setChecked(false);
    }
    else
    {
        xspin->setEnabled(true);
        yspin->setEnabled(true);
        arscheckbox->setEnabled(true);
    }

}
