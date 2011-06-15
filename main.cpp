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

#include "main.h"
#include <iostream>

QmfExplorer::QmfExplorer(QMainWindow* parent) : QMainWindow(parent)
{
    setupUi(this);

    qRegisterMetaType<qmf::Agent>();
    qRegisterMetaType<qmf::Data>();
    qRegisterMetaType<qmf::ConsoleEvent>();

    //
    // Setup some global app vales to be used by the QSettings class
    //
    QCoreApplication::setOrganizationName("Red Hat");
    QCoreApplication::setOrganizationDomain("redhat.com");
    QCoreApplication::setApplicationName("QMF-Explorer");

    //
    // Create the agent model which stores the list of known agents.
    //
    agentModel = new AgentModel(this);
    treeView_agents->setModel(agentModel);

    //
    // Create the agent-detail model which holds the attributes of the selected agent
    //
    agentDetail = new AgentDetailModel(this);
    tableView_agent_details->setModel(agentDetail);

    //
    // Create the object model which stores the list of queried objects.
    //
    objectModel = new ObjectModel(this);
    treeView_objects->setModel(objectModel);

    //
    // Create the object-detail model which holds the properties of an object.
    //
    objectDetail = new ObjectDetailModel(this);
    tableView_object->setModel(objectDetail);

    //
    // Create the event detail model to hold the event properties
    //
    eventDetail = new EventDetailModel(this);

    //
    // Ctrate a proxy model to enable sorting by column
    //
    eventtProxyModel = new QSortFilterProxyModel(this);
    eventtProxyModel->setSourceModel(eventDetail);

    tableView_events->setModel(eventtProxyModel);
    tableView_events->setSelectionBehavior(QAbstractItemView::SelectRows);

    //
    // Create the thread object that maintains communication with the messaging plane.
    //
    qmf = new QmfThread(this, agentModel, lineEdit_agent_filter, objectModel);
    qmf->start();

    //
    // Create the open connection dialog box
    //
    m_openDialog = new OpenDialog(this);
    connect(m_openDialog, SIGNAL(openDialogAccepted(QString,QString,QString)), qmf, SLOT(connect_url(QString,QString,QString)));

    //
    // Linkage for the menu and the Connection Status label.
    //
    connect(qmf, SIGNAL(connectionStatusChanged(QString)), label_connection_status, SLOT(setText(QString)));
    connect(actionOpen_Localhost, SIGNAL(triggered()), qmf, SLOT(connect_localhost()));
    connect(actionClose, SIGNAL(triggered()), qmf, SLOT(disconnect()));

    //
    // Linkage from the Agent Filter edit-box and button to the QMF thread
    //
    connect(pushButton_apply_filter, SIGNAL(pressed()), qmf, SLOT(applyAgentFilter()));

    //
    // Linkage for the Agent List tab components
    //
    connect(qmf,             SIGNAL(newAgent(qmf::Agent)),     agentModel,  SLOT(addAgent(qmf::Agent)));
    connect(qmf,             SIGNAL(delAgent(qmf::Agent)),     agentModel,  SLOT(delAgent(qmf::Agent)));
    connect(qmf,             SIGNAL(isConnected(bool)),        agentModel,  SLOT(clear()));
    connect(qmf,             SIGNAL(isConnected(bool)),        agentDetail, SLOT(clear()));
    connect(treeView_agents, SIGNAL(clicked(QModelIndex)),     agentModel,  SLOT(selected(QModelIndex)));
    connect(agentModel,      SIGNAL(instSelected(qmf::Agent)), agentDetail, SLOT(newAgent(qmf::Agent)));

    //
    // Linkage for Object tab components
    //
    connect(qmf, SIGNAL(newPackage(QString)), objectModel, SLOT(addPackage(QString)));
    connect(qmf, SIGNAL(newClass(QStringList)), objectModel, SLOT(addClass(QStringList)));
    connect(qmf, SIGNAL(addObject(qmf::Data)), objectModel, SLOT(addObject(qmf::Data)));
    connect(treeView_objects, SIGNAL(clicked(QModelIndex)), objectModel, SLOT(selected(QModelIndex)));
    connect(objectModel, SIGNAL(instSelected(qmf::Data)), objectDetail, SLOT(newObject(qmf::Data)));

    //
    // Linkage for the Event tab table
    //
    connect(qmf, SIGNAL(newEvent(qmf::ConsoleEvent)), eventDetail, SLOT(newEvent(qmf::ConsoleEvent)));
    connect(qmf, SIGNAL(newEvent(qmf::ConsoleEvent)), tableView_events, SLOT(resizeColumnsToContents()));

    //
    // Create linkages to enable and disable main-window components based on the connection status.
    //
    connect(qmf, SIGNAL(isConnected(bool)), tabWidget,            SLOT(setEnabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), actionOpen_Localhost, SLOT(setDisabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), actionOpen,           SLOT(setDisabled(bool)));
    connect(qmf, SIGNAL(isConnected(bool)), actionClose,          SLOT(setEnabled(bool)));
}

void QmfExplorer::init(int argc, char *argv[])
{
    QString url("localhost");
    QString connectionOptions;
    QString sessionOptions;

    if (argc > 1)
        url = QString(argv[1]);
    if (argc > 2)
        connectionOptions = QString(argv[2]);
    if (argc > 3)
        sessionOptions = QString(argv[3]);

    qmf->connect_url(url, connectionOptions, sessionOptions);
}

QmfExplorer::~QmfExplorer()
{
    qmf->cancel();
    qmf->wait();
    delete qmf;
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow *window = new QMainWindow;
    QmfExplorer qe(window);

    qe.show();
    qe.init(argc, argv);

    return app.exec();
}

void QmfExplorer::on_actionOpen_triggered()
{
    if (m_openDialog) {
        m_openDialog->exec();
    }
}
