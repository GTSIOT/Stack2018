
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

class ExampleListener : public virtual DDS::DataReaderListener
{
public:
    virtual void on_requested_deadline_missed (
        DDS::DataReader_ptr reader,
        const DDS::RequestedDeadlineMissedStatus & status)
    {
        std::cout << "on_requested_deadline_missed" << std::endl;
    }
    virtual void on_requested_incompatible_qos (
        DDS::DataReader_ptr reader,
        const DDS::RequestedIncompatibleQosStatus & status)
    {
        std::cout << "on_requested_incompatible_qos" << std::endl;
    }
    virtual void on_sample_rejected (
        DDS::DataReader_ptr reader,
        const DDS::SampleRejectedStatus & status)
    {
        std::cout << "on_sample_rejected" << std::endl;
    }
    virtual void on_liveliness_changed (
        DDS::DataReader_ptr reader,
        const DDS::LivelinessChangedStatus & status)
    {
        std::cout << "on_liveliness_changed" << std::endl;
    }
    virtual void on_data_available (
        DDS::DataReader_ptr reader)
    {
        std::cout << "on_data_available" << std::endl;
    }
    virtual void on_subscription_matched (
        DDS::DataReader_ptr reader,
        const DDS::SubscriptionMatchedStatus & status)
    {
        std::cout << "on_subscription_matched" << std::endl;
    }
    virtual void on_sample_lost (
        DDS::DataReader_ptr reader,
        const DDS::SampleLostStatus & status)
    {
        std::cout << "on_sample_lost" << std::endl;
    }
};

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
    DDS::Topic_var                    topicRain;   
    DDS::Topic_var                    topicTemperature;

    DDS::Subscriber_var               subscriberHumidity;
	DDS::Subscriber_var               subscriberRain;
    DDS::Subscriber_var               subscriberTemperature;

    DDS::DataReader_var               readerHumidity;
	DDS::DataReader_var               readerRain;
    DDS::DataReader_var               readerTemperature;
    
    EnvironmentalData::EnvironmentalDataReader_var  myReaderHumidity;   
	EnvironmentalData::EnvironmentalDataReader_var  myReaderRain;
    EnvironmentalData::EnvironmentalDataReader_var  myReaderTemperature;
 
    EnvironmentalData::EnvironmentalSeq msgListHumidity;
	EnvironmentalData::EnvironmentalSeq msgListRain;
    EnvironmentalData::EnvironmentalSeq msgListTemperature;

    DDS::SampleInfoSeq infoSeqHumidity;
	DDS::SampleInfoSeq infoSeqRain;
    DDS::SampleInfoSeq infoSeqTemperature;

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
	topicRain = participant->create_topic("rain", typeName, tQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(topicRain, "create_topic() rain failed");
    topicTemperature = participant->create_topic("temperature", typeName, tQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(topicTemperature, "create_topic() temperature failed");

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
	subscriberRain = participant->create_subscriber(sQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(subscriberRain, "create_subscriber() rain failed");
    subscriberTemperature = participant->create_subscriber(sQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(subscriberTemperature, "create_subscriber() temperature failed"); 

  // create DataReader entity
    DDS::DataReaderQos rQos;
    //result = subscriber->get_default_datareader_qos(rQos);
    result = qp.get_datareader_qos(rQos, NULL);
    checkStatus(result, "get_default_datareader_qos() failed");

    readerHumidity = subscriberHumidity->create_datareader(topicHumidity, rQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(readerHumidity, "create_datareader() humidity failed");
	 readerRain = subscriberRain->create_datareader(topicRain, rQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(readerRain, "create_datareader() rain failed");
    readerTemperature = subscriberTemperature->create_datareader(topicTemperature, rQos, new ExampleListener(), DDS::DATA_AVAILABLE_STATUS | DDS::REQUESTED_DEADLINE_MISSED_STATUS);
    checkHandle(readerTemperature, "create_datareader() temperature failed");

    /* Cast reader to 'HelloWorld' type specific interface. */
    myReaderHumidity = EnvironmentalData::EnvironmentalDataReader::_narrow(readerHumidity);
    checkHandle(myReaderHumidity, "EnvironmentalDataReader::_narrow() humidity failed");
    myReaderRain = EnvironmentalData::EnvironmentalDataReader::_narrow(readerRain);
    checkHandle(myReaderRain, "EnvironmentalDataReader::_narrow() temperature failed");
    myReaderTemperature = EnvironmentalData::EnvironmentalDataReader::_narrow(readerTemperature);
    checkHandle(myReaderTemperature, "EnvironmentalDataReader::_narrow() temperature failed");

    cout << "=== [Subscriber] Ready ..." << endl;
    return 0;
}

void Subscriberkill()
{
  // Delete all entities before termination (good practice to cleanup resources)
    result = subscriberHumidity->delete_datareader(readerHumidity);
    checkStatus(result, "delete_datareader() humidity failed");
    result = subscriberRain->delete_datareader(readerRain);
    checkStatus(result, "delete_datareader() rain failed");
    result = subscriberTemperature->delete_datareader(readerTemperature);
    checkStatus(result, "delete_datareader() temperature failed");

    result = participant->delete_subscriber(subscriberHumidity);
    checkStatus(result, "delete_publisher() humidity failed");
    result = participant->delete_subscriber(subscriberRain);
    checkStatus(result, "delete_publisher() rain failed");
    result = participant->delete_subscriber(subscriberTemperature);
    checkStatus(result, "delete_publisher() temperature failed");
    
    result = participant->delete_topic(topicHumidity);
    checkStatus(result, "delete_topic() humidity failed");
    result = participant->delete_topic(topicRain);
    checkStatus(result, "delete_topic() rain failed");
    result = participant->delete_topic(topicTemperature);
    checkStatus(result, "delete_topic() temperature failed");

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

bool RainRead()
{
  EnvironmentalData::EnvironmentalSeq msgList;
  DDS::SampleInfoSeq infoSeq;

  result = myReaderRain->take(msgList, infoSeq, DDS::LENGTH_UNLIMITED,
    DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  checkStatus(result, "EnvironmentalDataReader::take rain");
  
  msgListRain = msgList;
  infoSeqRain = infoSeq;

  result = myReaderRain->return_loan(msgList, infoSeq);
  checkStatus(result, "EnvironmentalDataReader::return_loan Rain");

   if(msgListRain.length() > 0)
   {
      return true;
   }else
   {
      return false;
   }
}

bool TemperatureRead()
{
  EnvironmentalData::EnvironmentalSeq msgList;
  DDS::SampleInfoSeq infoSeq;

  result = myReaderTemperature->take(msgList, infoSeq, DDS::LENGTH_UNLIMITED,
    DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  checkStatus(result, "EnvironmentalDataReader::take temperature");
  
  msgListTemperature = msgList;
  infoSeqTemperature = infoSeq;

  result = myReaderTemperature->return_loan(msgList, infoSeq);
  checkStatus(result, "EnvironmentalDataReader::return_loan temperature");

   if(msgListTemperature.length() > 0)
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

EnvironmentalData::EnvironmentalSeq RainGetDataSeq()
{
    return msgListRain;
}

EnvironmentalData::EnvironmentalSeq TemperatureGetDataSeq()
{
    return msgListTemperature;
}


DDS::SampleInfoSeq HumidityGetInfoSeq()
{
    return infoSeqHumidity;
}

DDS::SampleInfoSeq RainGetInfoSeq()
{
    return infoSeqRain;
}

DDS::SampleInfoSeq TemperatureGetInfoSeq()
{
    return infoSeqTemperature;
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

		 if(RainRead()){
            for (DDS::ULong i = 0; i < RainGetDataSeq().length(); ++i) {
                if (RainGetInfoSeq()[i].valid_data) {
                    float sensor_val = RainGetDataSeq()[i].value;
                    std::cout << "Rain: " << sensor_val << std::endl;
                }
            }
        }
		
        if(TemperatureRead()){
            for (DDS::ULong i = 0; i < TemperatureGetDataSeq().length(); ++i) {
                if (TemperatureGetInfoSeq()[i].valid_data) {
                    float sensor_val = TemperatureGetDataSeq()[i].value;
                    std::cout << "Temperature: " << sensor_val << std::endl;
                }
            }
        }
        
         os_nanoSleep(delay_100ms);
    }
    Subscriberkill();

    return 0;
}

