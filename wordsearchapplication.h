/***************************************************************************
 *   Copyright (C) 2013 M Wellings                                         *
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

#ifndef WORDSEARCHAPPLICATION_H
#define WORDSEARCHAPPLICATION_H

#include <QApplication>
#include <QSplashScreen>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QtNetwork>

class MainWindow;
class UpdateChecker;

class WordSearchApplication : public QApplication
{
    Q_OBJECT
public:
    explicit WordSearchApplication( int & argc, char **argv );
    bool event(QEvent *event);
    void registerWindow(MainWindow *window);
    void deRegisterWindow(MainWindow *window);
    QList<MainWindow*> windows;
    UpdateChecker *updateChecker;
private:
    MainWindow *window;
    void loadFile(const QString &fileName);
    QSplashScreen *Splash ;
signals:
    void windowListChanged();
public slots:
    void quitApplication();
    void updateWindowLists();
};

#endif // WORDSEARCHAPPLICATION_H
