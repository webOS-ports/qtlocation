/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativepinchgenerator_p.h"
#include "qdeclarativelocationtestmodel_p.h"

#include <QtQml/QQmlExtensionPlugin>
#include <QtQml/qqml.h>

#include <QDebug>

QT_BEGIN_NAMESPACE

class QLocationDeclarativeTestModule: public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")
public:
    virtual void registerTypes(const char* uri) {
        qDebug() << "registerTypes in new test plugin: " << uri;
        if (QLatin1String(uri) == QLatin1String("QtLocation.test")) {
            // This version numbering is not correct. It is just something to get going
            // until the proper versioning scheme of QML plugins in Qt5 is agreed upon.
            qmlRegisterType<QDeclarativePinchGenerator>(uri, 5, 0, "PinchGenerator");
            qmlRegisterType<QDeclarativeLocationTestModel>(uri, 5, 0, "TestModel");
        } else {
            qDebug() << "Unsupported URI given to load location test QML plugin: " << QLatin1String(uri);
        }
    }
};

#include "locationtest.moc"

QT_END_NAMESPACE

