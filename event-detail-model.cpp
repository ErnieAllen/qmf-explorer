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

    for (uint32_t idx = 0; idx < pcount; idx++) {
        qmf::Data d = event.getData(idx);

        const qpid::types::Variant::Map& attrs(d.getProperties());

        beginInsertRows(QModelIndex(), 0, attrs.size() - 1);
        for (qpid::types::Variant::Map::const_iterator iter = attrs.begin();
             iter != attrs.end(); iter++) {
                 // event.timestamp is in nano seconds, we need to convert to seconds
                 time_t ts = (time_t) event.getTimestamp() / 1000000000;
                 timeStamps << QString(ctime(&ts));

                 severities << QString::number(event.getSeverity());

                 QString prop = QString(iter->first.c_str());
                 prop += QString(": ");
                 prop += QString(iter->second.asString().c_str());
                 properties << prop;
         }
         endInsertRows();
    }

}


void EventDetailModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, timeStamps.size() - 1);
    timeStamps.clear();
    severities.clear();
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
    return 3;
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
    case 2: return properties.at(index.row());
    }
}


QVariant EventDetailModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    switch (section) {
    case 0: return QString("Time Stamp");
    case 1: return QString("Severity");
    case 2: return QString("Properties");
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

