/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -v -c GeoClue2Location -p geoclue2location_p.h:geoclue2location.cpp org.freedesktop.GeoClue2.xml org.freedesktop.GeoClue2.Location
 *
 * qdbusxml2cpp is Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef GEOCLUE2LOCATION_P_H_1400937813
#define GEOCLUE2LOCATION_P_H_1400937813

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.freedesktop.GeoClue2.Location
 */
class GeoClue2Location: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.GeoClue2.Location"; }

public:
    GeoClue2Location(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~GeoClue2Location();

    Q_PROPERTY(double Accuracy READ accuracy)
    inline double accuracy() const
    { return qvariant_cast< double >(property("Accuracy")); }

    Q_PROPERTY(double Altitude READ altitude)
    inline double altitude() const
    { return qvariant_cast< double >(property("Altitude")); }

    Q_PROPERTY(QString Description READ description)
    inline QString description() const
    { return qvariant_cast< QString >(property("Description")); }

    Q_PROPERTY(double Latitude READ latitude)
    inline double latitude() const
    { return qvariant_cast< double >(property("Latitude")); }

    Q_PROPERTY(double Longitude READ longitude)
    inline double longitude() const
    { return qvariant_cast< double >(property("Longitude")); }

public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
};

namespace org {
  namespace freedesktop {
    namespace GeoClue2 {
      typedef ::GeoClue2Location Location;
    }
  }
}
#endif
