/*
  qpegps is a program for displaying a map centered at the current longitude/
  latitude as read from a gps receiver.

  Copyright (C) 2002 Ralf Haselmeier <Ralf.Haselmeier@gmx.de>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
*/

#ifndef DIR_DIALOG_H
#define DIR_DIALOG_H
#include <qdialog.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qpushbutton.h>
#include "dirview.h"


class DirDialog : public QDialog
{
    Q_OBJECT
public:
    DirDialog(QWidget *, const char *, bool, WFlags);
    ~DirDialog();
    DirectoryView *directoryTree;
    QString selectedPath;
private slots:
    void selectPath(const QString &);
};


#endif
