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

#include "qmf-thread.h"
#include <qpid/messaging/exceptions.h>
#include <qmf/Query.h>

#include <iostream>
#include <string>

using std::cout;
using std::endl;

QmfThread::QmfThread(QObject* parent, AgentModel* agents, QLineEdit* f, ObjectModel* o) :
    QThread(parent), cancelled(false), connected(false), agentModel(agents), agentFilter(f), objectModel(o)
{
    // Intentionally Left Blank
}


void QmfThread::cancel()
{
    cancelled = true;
}


void QmfThread::connect_localhost()
{
    QMutexLocker locker(&lock);
    command_queue.push_back(Command(true, "localhost", "", "{strict-security:False}"));
    cond.wakeOne();
}

void QmfThread::connect_url(const QString& url, const QString& conn_options, const QString& qmf_options)
{
    QMutexLocker locker(&lock);
    command_queue.push_back((Command(true, url.toStdString(),
                                     conn_options.toStdString(),
                                     qmf_options.toStdString())));
    cond.wakeOne();
}

void QmfThread::disconnect()
{
    QMutexLocker locker(&lock);
    command_queue.push_back(Command(false, "", "", ""));
    cond.wakeOne();
}


void QmfThread::applyAgentFilter()
{
    if (connected)
        try {
            sess.setAgentFilter(agentFilter->text().toStdString());
        } catch (qmf::QmfException& e) {
            cout << "Exception: " << e.what() << endl;
        }
}


void QmfThread::run()
{
    emit connectionStatusChanged("Closed");

    while(true) {
        if (connected) {
            qmf::ConsoleEvent event;
            uint32_t pcount;
            int i;

            if (sess.nextEvent(event, qpid::messaging::Duration::SECOND)) {
                //
                // Process the event
                //
                qmf::Agent agent = event.getAgent();
                switch (event.getType()) {
                case qmf::CONSOLE_AGENT_ADD :
                    emit newAgent(agent);
                    agent.querySchemaAsync();
                    break;

                case qmf::CONSOLE_AGENT_DEL :
                    emit delAgent(agent);
                    break;

                case qmf::CONSOLE_AGENT_SCHEMA_UPDATE :
                    agent.querySchemaAsync();
                    break;

                case qmf::CONSOLE_AGENT_SCHEMA_RESPONSE :
                    // The agent schema response is coming in as
                    // an query response. This is a bug
                case qmf::CONSOLE_QUERY_RESPONSE :
                    // Handle the agent schema response
                    pcount = event.getSchemaIdCount();
                    for (uint32_t idx = 0; idx < pcount; idx++) {
                        agent.queryAsync(qmf::Query(qmf::QUERY_OBJECT, event.getSchemaId(idx)));
                    }

                    // Handle the query response
                    pcount = event.getDataCount();
                    for (uint32_t idx = 0; idx < pcount; idx++) {
                        emit addObject(event.getData(idx));
                    }

                    break;

                case qmf::CONSOLE_METHOD_RESPONSE :
                    i=2;
                    break;

                case qmf::CONSOLE_EVENT :
                    emit newEvent(event);
                    break;
                default :
                    break;
                }

            }

            {
                QMutexLocker locker(&lock);
                if (command_queue.size() > 0) {
                    Command command(command_queue.front());
                    command_queue.pop_front();
                    if (!command.connect) {
                        emit connectionStatusChanged("QMF Session Closing...");
                        sess.close();
                        emit connectionStatusChanged("Closing...");
                        conn.close();
                        emit connectionStatusChanged("Closed");
                        connected = false;
                        emit isConnected(false);
                    }
                }
            }
        } else {
            QMutexLocker locker(&lock);
            if (command_queue.size() == 0)
                cond.wait(&lock, 1000);
            if (command_queue.size() > 0) {
                Command command(command_queue.front());
                command_queue.pop_front();
                if (command.connect & !connected)
                    try {
                        emit connectionStatusChanged("QMF connection opening...");

                        conn = qpid::messaging::Connection(command.url, command.conn_options);
                        conn.open();

                        emit connectionStatusChanged("QMF session opening...");
                        sess = qmf::ConsoleSession(conn, command.qmf_options);
                        sess.open();
                        try {
                            sess.setAgentFilter("[eq, _product, [quote, 'qpidd']]");

                            //sess.setAgentFilter(agentFilter->text().toStdString());
                        } catch (std::exception&) {}
                        connected = true;
                        emit isConnected(true);

                        std::stringstream line;
                        line << "Operational (URL: " << command.url << ")";
                        emit connectionStatusChanged(line.str().c_str());
                    } catch(qpid::messaging::MessagingException& ex) {
                        std::stringstream line;
                        line << "QMF Session Failed: " << ex.what();
                        emit connectionStatusChanged(line.str().c_str());
                    }
            }
        }

        if (cancelled) {
            if (connected) {
                sess.close();
                conn.close();
            }
            break;
        }
    }
}

