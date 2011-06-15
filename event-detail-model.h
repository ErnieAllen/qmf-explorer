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

#ifndef EVENTDETAILMODEL_H
#define EVENTDETAILMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QStringList>
#include <qmf/Data.h>
#include <qmf/ConsoleEvent.h>
#include <sstream>
#include <string>
#include <stdint.h>

Q_DECLARE_METATYPE(qmf::ConsoleEvent);

class EventDetailModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit EventDetailModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex parent(const QModelIndex& index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

public slots:
    void newEvent(const qmf::ConsoleEvent&);
    void clear();

private:
    QStringList timeStamps;
    QStringList severities;
    QStringList names;
    QStringList properties;

};

#endif // EVENTDETAILMODEL_H
