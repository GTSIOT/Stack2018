
/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2017 PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

/************************************************************************
 * LOGICAL_NAME:    HelloWorldDataSubscriber.cpp
 * FUNCTION:        OpenSplice HelloWorld example code.
 * MODULE:          HelloWorld for the C++ programming language.
 * DATE             June 2017.
 * AUTHOR:          R.C.G. Poth
 *
 * Description:
 *
 * This file contains the implementation for the 'HelloWorldDataSubscriber'
 * executable. The goal is to provide a simple example of how to publish
 * data.
 *
 * This executable:
 * - creates the entities required to publish a message in the Domain
 * - publishes the HelloWorld message
 * - delete all entities
 * - terminate execution
 *
 ***/
#include <iostream>
#include "ccpp_dds_dcps.h"        /* Include the DDS::DCPS API */
#include "ccpp_EnvironmentalData.h"  /* Include the generated type specific (EnvironmentalData) DCPS API */
#include "example_main.h"         /* Include to define the application main() wrapper OSPL_MAIN. */
#include "QosProvider.h"
using namespace std;

/**
 * Check the return status for errors.
 * If there is an error, then report info message and terminate.
 **/
static void checkStatus(DDS::ReturnCode_t status, const char *info);

/**
 * Check whether a valid handle has been returned.
 * If not, then report info message and terminate.
 **/
static void checkHandle(void *handle, string info);

/* entry point exported and demangled so symbol can be found in shared library */
extern "C"
{
  OS_API_EXPORT
  int EnvironmentalDataSubscriber(int argc, char *argv[]);
}

/* Global data variables */
    DDS::DomainParticipantFactory_var factory;
    DDS::DomainId_t                   domain;
    DDS::DomainParticipant_var        participant;

    DDS::Topic_var                    topicHumidity;

    DDS::Subscriber_var               subscriberHumidity;

    DDS::DataReader_var               readerHumidity;
    
    EnvironmentalData::EnvironmentalDataReader_var  myReaderHumidity;   
    
    EnvironmentalData::EnvironmentalSeq msgListHumidity;
    
    DDS::SampleInfoSeq infoSeqHumidity;
    
    DDS::ReturnCode_t result;
/*
 * The main function of the Subscriber application
 */
int EnvironmentalDataSubscriber(int argc, char *argv[])
{
    /* The DDS entities required to publish data */

    //=======Load Qos Policy file======
    DDS::QosProvider qp("file://DDS_DefaultQoS.xml", "DefaultQosProfile");

    /* The Application EnvironmentalData Data TypeSupport */
    EnvironmentalData::EnvironmentalTypeSupport_var typesupport;
    DDS::String_var typeName;

  // Get the DDS DomainParticipantFactory
    factory = DDS::DomainParticipantFactory::get_instance();
    checkHandle(factory, "get_instance() failed");

  // Create a domain participant entity for the Default Domain (Domain Id = 0)
    domain = DDS::DOMAIN_ID_DEFAULT;
    participant = factory->create_participant(domain, PARTICIPANT_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(participant, "create_participant() failed");

  // Register the application data type
    typesupport = new EnvironmentalData::EnvironmentalTypeSupport();
    /* Get the IDL type name and use this to register the type. */
    typeName = typesupport->get_type_name();
    result = typesupport->register_type(participant, typeName);
    checkStatus(result, "register_type() failed");

  // Create Topic entity
    /* Create and initialize topic qos value on heap. */
    DDS::TopicQos tQos;
    //result = participant->get_default_topic_qos(tQos);
    result = qp.get_topic_qos(tQos, NULL);
    checkStatus(result, "get_default_topic_qos() failed");

    /* Fine tune topic qos, i.e. make topic reliable and transient (for late joining subscribers) */
    //tQos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
    //tQos.durability.kind = DDS::TRANSIENT_DURABILITY_QOS;
    /* Use the changed policy when defining the EnvironmentalData topic */
    topicHumidity = participant->create_topic("humidity", typeName, tQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(topicHumidity, "create_topic() humidity failed");
    
  // Create Subscriber entity
    /* Create on heap and initialize subscriber qos value with the default value. */
    DDS::SubscriberQos sQos;
    //result = participant->get_default_subscriber_qos(sQos);
    result = qp.get_subscriber_qos(sQos, NULL);
    checkStatus(result, "get_default_subscriber_qos() failed");

    /* Fine tune the partition qos policy ito the partition from which the data will be received. */
    //sQos.partition.name.length(1);
    //sQos.partition.name[0] = "EnvironmentalData Partition";
    /* Create the subscriber. */
    subscriberHumidity = participant->create_subscriber(sQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(subscriberHumidity, "create_subscriber() humidity failed");
    
  // create DataReader entity
    DDS::DataReaderQos rQos;
    //result = subscriber->get_default_datareader_qos(rQos);
    result = qp.get_datareader_qos(rQos, NULL);
    checkStatus(result, "get_default_datareader_qos() failed");

    readerHumidity = subscriberHumidity->create_datareader(topicHumidity, rQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(readerHumidity, "create_datareader() humidity failed");
    
    /* Cast reader to 'HelloWorld' type specific interface. */
    myReaderHumidity = EnvironmentalData::EnvironmentalDataReader::_narrow(readerHumidity);
    checkHandle(myReaderHumidity, "EnvironmentalDataReader::_narrow() humidity failed");
    
    cout << "=== [Subscriber] Ready ..." << endl;
    return 0;
}

void Subscriberkill()
{
  // Delete all entities before termination (good practice to cleanup resources)
    result = subscriberHumidity->delete_datareader(readerHumidity);
    checkStatus(result, "delete_datareader() humidity failed");
    
    result = participant->delete_subscriber(subscriberHumidity);
    checkStatus(result, "delete_publisher() humidity failed");
    
    result = participant->delete_topic(topicHumidity);
    checkStatus(result, "delete_topic() humidity failed");
    
    result = factory->delete_participant(participant);
    checkStatus(result, "delete_participant() failed");
}

bool HumidityRead()
{
  EnvironmentalData::EnvironmentalSeq msgList;
  DDS::SampleInfoSeq infoSeq;

  result = myReaderHumidity->take(msgList, infoSeq, DDS::LENGTH_UNLIMITED,
    DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  checkStatus(result, "EnvironmentalDataReader::take humidity");
  
  msgListHumidity = msgList;
  infoSeqHumidity = infoSeq;

  result = myReaderHumidity->return_loan(msgList, infoSeq);
  checkStatus(result, "EnvironmentalDataReader::return_loan humidity");

   if(msgListHumidity.length() > 0)
   {
      return true;
   }else
   {
      return false;
   }
}

EnvironmentalData::EnvironmentalSeq HumidityGetDataSeq()
{
    return msgListHumidity;
}


DDS::SampleInfoSeq HumidityGetInfoSeq()
{
    return infoSeqHumidity;
}

/* End of the Subscriber  example application.
 * Following are the implementation of error checking helper function.
 */

/* Array to hold the names for all ReturnCodes. */
string RetCodeName[13] =
{
    "DDS_RETCODE_OK", "DDS_RETCODE_ERROR", "DDS_RETCODE_UNSUPPORTED",
    "DDS_RETCODE_BAD_PARAMETER", "DDS_RETCODE_PRECONDITION_NOT_MET",
    "DDS_RETCODE_OUT_OF_RESOURCES", "DDS_RETCODE_NOT_ENABLED",
    "DDS_RETCODE_IMMUTABLE_POLICY", "DDS_RETCODE_INCONSISTENT_POLICY",
    "DDS_RETCODE_ALREADY_DELETED", "DDS_RETCODE_TIMEOUT", "DDS_RETCODE_NO_DATA",
    "DDS_RETCODE_ILLEGAL_OPERATION"
};

/**
 * Check the return status for errors. If there is an error, then terminate.
 **/
static void checkStatus(DDS::ReturnCode_t status, const char *info)
{
    if (status != DDS::RETCODE_OK && status != DDS::RETCODE_NO_DATA) {
        cerr << "Error in " << info << "with return code : " << RetCodeName[status].c_str() << endl;
        exit(1);
    }
}

/**
 * Check whether a valid handle has been returned. If not, then terminate.
 **/
static void checkHandle(void *handle, string info)
{
    if (!handle) {
        cerr << "Error in " << info.c_str() << ": Creation failed: invalid handle" << endl;
        exit(1);
    }
}

/* Main wrapper to allow embedded usage of the Subscriber application. */
int OSPL_MAIN (int argc, char *argv[])
{
  os_time delay_100ms = { 0, 100000000 }; //100ms
  EnvironmentalDataSubscriber (argc, argv);

for(;;){

        if(HumidityRead()){
            for (DDS::ULong i = 0; i < HumidityGetDataSeq().length(); ++i) {
                if (HumidityGetInfoSeq()[i].valid_data) {
                    float sensor_val = HumidityGetDataSeq()[i].value;
                    std::cout << "Humi: " << sensor_val << std::endl;
                }
            }
        }
        
         os_nanoSleep(delay_100ms);
    }
    Subscriberkill();

    return 0;
}

