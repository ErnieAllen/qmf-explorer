/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "event-detail-model.h"
#include <iostream>
#include <sstream>
#include <QDateTime>
#include "qpid/sys/Time.h"

using std::cout;
using std::endl;

EventDetailModel::EventDetailModel(QObject *parent) : QAbstractItemModel(parent)
{
    // Intentionally Left Blank
}

void EventDetailModel::newEvent(const qmf::ConsoleEvent& event)
{
    uint32_t pcount = event.getDataCount();
    if (pcount < 1)
        return;

    beginInsertRows(QModelIndex(), 0, pcount - 1);
    // each data in event is a new row
    for (uint32_t idx = 0; idx < pcount; idx++) {
        qmf::Data d = event.getData(idx);

        const qpid::types::Variant::Map& attrs(d.getProperties());

        // event.timestamp is in nano seconds, we need to convert to seconds
        time_t ts = (time_t) event.getTimestamp() / 1000000000;
        timeStamps << QString(ctime(&ts));

        QString sevName;
        switch (event.getSeverity()) {
        case qmf::SEV_EMERG  : sevName = "EMERG";    break;
        case qmf::SEV_ALERT  : sevName = "ALERT";    break;
        case qmf::SEV_CRIT   : sevName = "CRITICAL"; break;
        case qmf::SEV_ERROR  : sevName = "ERROR";    break;
        case qmf::SEV_WARN   : sevName = "WARN";     break;
        case qmf::SEV_NOTICE : sevName = "NOTICE";   break;
        case qmf::SEV_INFORM : sevName = "INFO";     break;
        }

        severities << sevName;
        QString name(d.getSchemaId().getPackageName().c_str());
        name += ":";
        name += d.getSchemaId().getName().c_str();
        names << name;

        QString prop;
        bool first = true;

        // each attribute in the data is concatenated in the properties column
        for (qpid::types::Variant::Map::const_iterator iter = attrs.begin();
             iter != attrs.end(); iter++) {
            if (first)
                first = false;
            else
                prop += QString(", ");
            prop += QString(iter->first.c_str());
            prop += QString("=");
            prop += QString(iter->second.asString().c_str());
        }
        properties << prop;
    }
    endInsertRows();
}


void EventDetailModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, timeStamps.size() - 1);
    timeStamps.clear();
    severities.clear();
    names.clear();
    properties.clear();
    endRemoveRows();
}


int EventDetailModel::rowCount(const QModelIndex &parent) const
{
    //
    // If the parent is invalid (top-level), return the number of attributes.
    //
    if (!parent.isValid())
        return (int) timeStamps.size();

    //
    // This is not a tree so there are not child rows.
    //
    return 0;
}


int EventDetailModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}


QVariant EventDetailModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (!index.isValid())
        return QVariant();

    switch (index.column()) {
    case 0: return timeStamps.at(index.row());
    case 1: return severities.at(index.row());
    case 2: return names.at(index.row());
    case 3: return properties.at(index.row());
    }
}


QVariant EventDetailModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    switch (section) {
    case 0: return QString("Time Stamp");
    case 1: return QString("Severity");
    case 2: return QString("Name");
    case 3: return QString("Properties");
    }

    return QVariant();
}


QModelIndex EventDetailModel::parent(const QModelIndex& index) const
{
    //
    // Not a tree structure, no parents.
    //
    return QModelIndex();
}


QModelIndex EventDetailModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column);

    return QModelIndex();
}

