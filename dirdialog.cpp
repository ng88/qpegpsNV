/*
  qpegps is a program for displaying a map centered at the current longitude/
  latitude as read from a gps receiver.

  qpeGPS NV >= 1.1 with route navigation Copyright (C) 2006 Nicolas Guillaume <ng@ngsoft-fr.com>
  qpeGPS <= 0.9.2.3.3 Copyright (C) 2002 Ralf Haselmeier <Ralf.Haselmeier@gmx.de>
 
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


#include "dirdialog.h"

#include <qfileinfo.h>
#include <qdir.h>
#include "dirview.h"

/* improved by ng */

DirDialog::DirDialog(QWidget * parent = 0, const char *name = 0, bool modal = FALSE, WFlags f = 0)
 : QDialog(parent, name, modal, f)
{
    resize(parent->geometry().size());
    
    DirectoryView * directoryTree = new DirectoryView(this, 0, TRUE);
    directoryTree->addColumn(tr("Name"));
    //directoryTree->addColumn( "Type" );
    directoryTree->setTreeStepSize(20);

    const QFileInfoList *roots = QDir::drives();
    QListIterator<QFileInfo> i(*roots);
    QFileInfo *fi;
    while ((fi = *i))
    {
        ++i;
        Directory *root = new Directory(directoryTree, fi->filePath());
        if (roots->count() <= 1)
            root->setOpen(true);        // be interesting
    }
    
    directoryTree->setAllColumnsShowFocus(true);
    directoryTree->resize(geometry().size());
    
    connect(directoryTree, SIGNAL(folderSelected(const QString &)),
            this, SLOT(selectPath(const QString &)));
}

DirDialog::~DirDialog()
{
}

void DirDialog::selectPath(const QString & s)
{
    _selectedPath = s;
}
