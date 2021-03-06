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
 * LOGICAL_NAME:    EnvironmentalDataPublisher.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE:            June 2017.
 * AUTHOR:          R.C.G. Poth
 *
 * Description:
 *
 * This file contains the implementation for the 'EnvironmentalDataPublisher'
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
#include <random>
#include "ccpp_dds_dcps.h"        /* Include the DDS::DCPS API */
#include "ccpp_EnvironmentalData.h"  /* Include the generated type specific (EnvironmentalData) DCPS API */
#include "example_main.h"         /* Include to define the application main() wrapper OSPL_MAIN. */
#include "QosProvider.h"
//#include <SerialStream.h>

using namespace std;
//using namespace LibSerial;

#define SERIAL_PORT  "/dev/ttyS1"
#define SERIAL_BAUD 115200

struct Sensors{
    float temperature;
    float humidity;
    float rain;
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
  int EnvironmentalDataPublisher(int argc, char *argv[]);
}

/* Global data variables */
    DDS::DomainParticipantFactory_var factory;
    DDS::DomainId_t                   domain;
    DDS::DomainParticipant_var        participant;

    DDS::Topic_var                    topicHumidity;
    DDS::Topic_var                    topicTemperature;
    DDS::Topic_var                    topicRain;    

    DDS::Publisher_var                publisherHumidity;
    DDS::Publisher_var                publisherTemperature;  
    DDS::Publisher_var                publisherRain;

    DDS::DataWriter_var               writerHumidity;
    DDS::DataWriter_var               writerTemperature;
    DDS::DataWriter_var               writerRain;
    
    EnvironmentalData::EnvironmentalDataWriter_var  myWriterHumidity;   
    EnvironmentalData::EnvironmentalDataWriter_var  myWriterTemperature;
    EnvironmentalData::EnvironmentalDataWriter_var  myWriterRain;

    DDS::ReturnCode_t result;
/*
 * The main function of the Publisher application
 */
int EnvironmentalDataPublisher (int argc, char *argv[])
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
	
    topicTemperature = participant->create_topic("temperature", typeName, tQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(topicTemperature, "create_topic() temperature failed");
    
	topicRain = participant->create_topic("rain", typeName, tQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(topicRain, "create_topic() rain failed");

  // Create Publisher entity
    /* Create on heap and initialize publisher qos value with the default value. */
    DDS::PublisherQos pQos;
    //result = participant->get_default_publisher_qos(pQos);
    result = qp.get_publisher_qos(pQos, NULL);
    checkStatus(result, "get_default_publisher_qos() failed");

    /* Fine tune the partition qos policy ito the partition in which the data will be published. */
    //pQos.partition.name.length(1);
    //pQos.partition.name[0] = "EnvironmentalData Partition";
    /* Create the publisher. */
    publisherHumidity = participant->create_publisher(pQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(publisherHumidity, "create_publisher() humidity failed");
	
    publisherTemperature = participant->create_publisher(pQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(publisherTemperature, "create_publisher() temperature failed");
    
	publisherRain = participant->create_publisher(pQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(publisherRain, "create_publisher() rain failed");

  // create DataWriter entity
    DDS::DataWriterQos wQos;
    result = qp.get_datawriter_qos(wQos, NULL);
    checkStatus(result, "get_default_datawriter_qos() failed");

    /* Set the autodispose_unregistered_instances qos policy to false.
     * If autodispose_unregistered_instances is set to true (default value),
     * you will have to start the subscriber before the publisher
     */
    //wQos.writer_data_lifecycle.autodispose_unregistered_instances = false;

    writerHumidity = publisherHumidity->create_datawriter(topicHumidity, wQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(writerHumidity, "create_datawriter() humidity failed");
    
	writerTemperature = publisherTemperature->create_datawriter(topicTemperature, wQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(writerTemperature, "create_datawriter() temperature failed");
    
	writerRain = publisherRain->create_datawriter(topicRain, wQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(writerRain, "create_datawriter() rain failed");

    /* Cast writer to 'EnvironmentalData' type specific interface. */
    myWriterHumidity = EnvironmentalData::EnvironmentalDataWriter::_narrow(writerHumidity);
    checkHandle(myWriterHumidity, "EnvironmentalDataWriter::_narrow() humidity failed");
    
	myWriterTemperature = EnvironmentalData::EnvironmentalDataWriter::_narrow(writerTemperature);
    checkHandle(myWriterTemperature, "EnvironmentalDataWriter::_narrow() temperature failed");
    
	myWriterRain = EnvironmentalData::EnvironmentalDataWriter::_narrow(writerRain);
    checkHandle(myWriterRain, "EnvironmentalDataWriter::_narrow() rain failed");

    cout << "=== [Publisher Fake] Ready ..." << endl;
    return 0;
}

void HumidityPublish(EnvironmentalData::Environmental& instance)
{
  result = myWriterHumidity->write(instance, DDS::HANDLE_NIL);
  checkStatus(result, "SensorDataWriter::write humidity");
}

void TemperaturePublish(EnvironmentalData::Environmental& instance)
{
  result = myWriterTemperature->write(instance, DDS::HANDLE_NIL);
  checkStatus(result, "SensorDataWriter::write temperature");
}

void RainPublish(EnvironmentalData::Environmental& instance)
{
  result = myWriterRain->write(instance, DDS::HANDLE_NIL);
  checkStatus(result, "SensorDataWriter::write rain");
}

void PublisherKill()
{
  // Delete all entities before termination (good practice to cleanup resources)
    result = publisherHumidity->delete_datawriter(writerHumidity);
    checkStatus(result, "delete_datawriter() humidity failed");
    result = publisherTemperature->delete_datawriter(writerTemperature);
    checkStatus(result, "delete_datawriter() temperature failed");
    result = publisherRain->delete_datawriter(writerRain);
    checkStatus(result, "delete_datawriter() rain failed");

    result = participant->delete_publisher(publisherHumidity);
    checkStatus(result, "delete_publisher() failed");
    result = participant->delete_publisher(publisherTemperature);
    checkStatus(result, "delete_publisher() failed");
    result = participant->delete_publisher(publisherRain);
    checkStatus(result, "delete_publisher() failed");

    result = participant->delete_topic(topicHumidity);
    checkStatus(result, "delete_topic() humidity failed");
    result = participant->delete_topic(topicTemperature);
    checkStatus(result, "delete_topic() temperature failed");
    result = participant->delete_topic(topicRain);
    checkStatus(result, "delete_topic() rain failed");

    result = factory->delete_participant(participant);
    checkStatus(result, "delete_participant() failed");
}

/* End of the Publisher example application.
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

std::string create_id(char machine_id[20], char* node_id, int sensor_id, char *type){
    std::string id;
    
    id.append(machine_id);
    id.append("N");
    id.append(node_id);
    id.append("S");
    id.append(std::to_string(sensor_id));
    id.append(type);
    
    return id;
}

float get_humi(){
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> val(68.0, 70.0); 
    
    return (float)val(rng) * 1.02;
}

float get_temp(){
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> val(25.01,27.98); 
    
    return (float)val(rng) * 1.02;
}

float get_rain(){
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> val(0,1); 
    
    return (float)val(rng);
}

/* Main wrapper to allow embedded usage of the Publisher application. */
int OSPL_MAIN (int argc, char *argv[])
{
    char MACHINE_ID[20];
    gethostname(MACHINE_ID, 20);

    char* NODE_ID;
    NODE_ID = argv[1];
    if(NODE_ID == NULL){
        std::cout << "ERROR: NODE ID MISSING! EXITING NOW..."<< std::endl;
        return 1;
    }

   os_time delay_100ms = { 0, 100000000 }; //100ms

   EnvironmentalDataPublisher(argc, argv);

    std::string humi_id = create_id(MACHINE_ID, NODE_ID, 0, (char*)"hum");
    std::string temp_id = create_id(MACHINE_ID, NODE_ID, 1, (char*)"tem");
    std::string rain_id = create_id(MACHINE_ID, NODE_ID, 2, (char*)"rai");

    EnvironmentalData::Environmental humi_instance;
    humi_instance.id = DDS::String_mgr(humi_id.c_str());
    humi_instance.type = DDS::String_mgr("humidity sensor");

    EnvironmentalData::Environmental temp_instance;
    temp_instance.id = DDS::String_mgr(temp_id.c_str());
    temp_instance.type = DDS::String_mgr("temperature sensor");

    EnvironmentalData::Environmental rain_instance;
    rain_instance.id = DDS::String_mgr(rain_id.c_str());
    rain_instance.type = DDS::String_mgr("rain sensor");

    for(;;){

        humi_instance.value = get_humi();
        HumidityPublish(humi_instance); 

        temp_instance.value = get_temp();
        TemperaturePublish(temp_instance); 

        rain_instance.value = get_rain();
        RainPublish(rain_instance); 

        //NDDSUtility::sleep(send_period);
        os_nanoSleep(delay_100ms);
    }

    PublisherKill();

    return 0;
}

