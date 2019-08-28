#include <uuid/uuid.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <algorithm>
#include "rdkafka.h"
#include <cpp_redis/cpp_redis>
#include <tacopie/tacopie>
#include "../sb-loader/sb_loader/lib/SBReadFile.h"



void ConvertSBImages(const std::string & filename);

static int run = 1;


static void stop (int sig) {
    run = 0;
    fclose(stdin);

}

static void dr_msg_cb (rd_kafka_t *rk,
                       const rd_kafka_message_t *rkmessage, void *opaque) {
    if (rkmessage->err)
            fprintf(stderr, "%% Message delivery failed: %s\n",
                    rd_kafka_err2str(rkmessage->err));
    else
            fprintf(stderr,
                    "%% Message delivered (%zd bytes, "
                    "partition %" PRId32 ")\n",
                    rkmessage->len, rkmessage->partition);

    /* The rkmessage is destroyed automatically by librdkafka */
}

int main(int argc, char ** argv)
{
    rd_kafka_t *rk;         /* Producer instance handle */
    rd_kafka_topic_t *rkt;  /* Topic object */
    rd_kafka_conf_t *conf;  /* Temporary configuration object */
    char errstr[512];       /* librdkafka API error reporting buffer */
    const char *brokers;    /* Argument: broker list */
    const char *topic;      /* Argument: topic to produce to */
    const char *path;

    /*
     * Argument validation
     */
    /*if (argc != 4) {
            fprintf(stderr, "%% Usage: %s <broker> <topic> <path>\n", argv[0]);
            return 1;
    }*/

    brokers = "localhost:9092";
    topic   = "my-test-1";
    path = "/home/faceman/UQ/thesis/slidebook/MB231_gfp-Lifeact_01.sld";


    /*
     * Create Kafka client configuration place-holder
     */
    conf = rd_kafka_conf_new();

    /* Set bootstrap broker(s) as a comma-separated list of
     * host or host:port (default port 9092).
     * librdkafka will use the bootstrap brokers to acquire the full
     * set of brokers from the cluster. */
    if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers,
                          errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
            fprintf(stderr, "%s\n", errstr);
            return 1;
    }

    /* Set the delivery report callback.
     * This callback will be called once per message to inform
     * the application if delivery succeeded or failed.
     * See dr_msg_cb() above. */
    rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);


    /*
     * Create producer instance.
     *
     * NOTE: rd_kafka_new() takes ownership of the conf object
     *       and the application must not reference it again after
     *       this call.
     */
    rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if (!rk) {
            fprintf(stderr,
                    "%% Failed to create new producer: %s\n", errstr);
            return 1;
    }


    /* Create topic object that will be reused for each message
     * produced.
     *
     * Both the producer instance (rd_kafka_t) and topic objects (topic_t)
     * are long-lived objects that should be reused as much as possible.
     */
    rkt = rd_kafka_topic_new(rk, topic, NULL);
    if (!rkt) {
            fprintf(stderr, "%% Failed to create topic object: %s\n",
                    rd_kafka_err2str(rd_kafka_last_error()));
            rd_kafka_destroy(rk);
            return 1;
    }

    /* Signal handler for clean shutdown */
    signal(SIGINT, stop);

    cpp_redis::client client;
    client.connect("127.0.0.1",6379,[](const std::string& host, std::size_t port, cpp_redis::client::connect_state status) {
        if (status == cpp_redis::client::connect_state::dropped) {
            std::cout<<"client disconnected from"<<host<<":"<<port<< std::endl;
        }
    });



    III::SBReadFile* sb_read_file = III_NewSBReadFile(path, III::kNoExceptionsMasked);

    CaptureIndex number_captures = sb_read_file->GetNumCaptures();

    for (int capture_index = 0; capture_index < number_captures; capture_index++)
    {

        PositionIndex number_channels = sb_read_file->GetNumChannels(capture_index);
        ChannelIndex number_positions = sb_read_file->GetNumPositions(capture_index);
        TimepointIndex number_timepoints = sb_read_file->GetNumTimepoints(capture_index);

        UInt32 xDim = sb_read_file->GetNumXColumns(capture_index);
        UInt32 yDim = sb_read_file->GetNumYRows(capture_index);
        UInt32 zDim = sb_read_file->GetNumZPlanes(capture_index);
        std::string dataInfo = "info;"+std::to_string(xDim)+";"
                + std::to_string(yDim) + ";"
                + std::to_string(zDim) +";"
                + std::to_string(number_channels);

        char dataInfo_c[dataInfo.size()+1];
        strcpy(dataInfo_c, dataInfo.c_str());

        bool has_voxel_size = false;
        float voxel_size[3];
        has_voxel_size = sb_read_file -> GetVoxelSize(capture_index,voxel_size[0], voxel_size[1], voxel_size[2]);
        if(!has_voxel_size) {
            voxel_size[0] = voxel_size[1] = voxel_size[2] = 1.0;
        }

        size_t planeSize = xDim * yDim;
        int bufferSize = xDim * yDim * zDim;
        int bufferSizeInBytes = bufferSize&sizeof(UInt16);
        UInt16* buffer = new UInt16[bufferSize];

        UInt16* planeBuffer = new UInt16[planeSize];

        for(int timepoint_index = 0; timepoint_index < number_timepoints; timepoint_index++) {
            for(int channel = 0; channel < number_channels; channel++) {
                for(int z = 0; z < zDim; z++) {
                    sb_read_file->ReadImagePlaneBuf(planeBuffer, capture_index, 0, timepoint_index, z, channel);
                    uuid_t uuid;
                    uuid_generate(uuid);
                    char key[36];
                    uuid_unparse(uuid,key);
                    const std::string planeBuffer_s = std::to_string((long)planeBuffer);

                    //send it to Redis with UUID and the planeBuffer n
                    client.set(key,planeBuffer_s);

                    if(z == 0) {
                        rd_kafka_produce(
                                    rkt,
                                    RD_KAFKA_PARTITION_UA,
                                    RD_KAFKA_MSG_F_COPY,
                                    dataInfo_c, strlen(dataInfo_c),
                                    NULL, 0,
                                    NULL
                                    );
                    }
                    retry: if (rd_kafka_produce(
                                /* Topic object */
                                rkt,
                                /* Use builtin partitioner to select partition*/
                                RD_KAFKA_PARTITION_UA,
                                /* Make a copy of the payload. */
                                RD_KAFKA_MSG_F_COPY,
                                /* Message payload (value) and length */
                                key, strlen(key),
                                /* Optional key and its length */
                                NULL, 0,
                                /* Message opaque, provided in
                                 * delivery report callback as
                                 * msg_opaque. */
                                NULL) == -1) {
                            /**
                             * Failed to *enqueue* message for producing.
                             */
                            fprintf(stderr,
                                    "%% Failed to produce to topic %s: %s\n",
                                    rd_kafka_topic_name(rkt),
                                    rd_kafka_err2str(rd_kafka_last_error()));

                            /* Poll to handle delivery reports */
                            if (rd_kafka_last_error() ==
                                RD_KAFKA_RESP_ERR__QUEUE_FULL) {
                                    /* If the internal queue is full, wait for
                                     * messages to be delivered and then retry.
                                     * The internal queue represents both
                                     * messages to be sent and messages that have
                                     * been sent or failed, awaiting their
                                     * delivery report callback to be called.
                                     *
                                     * The internal queue is limited by the
                                     * configuration property
                                     * queue.buffering.max.messages */
                                    rd_kafka_poll(rk, 1000/*block for max 1000ms*/);
                                    goto retry;
                            }
                    }
                }
            }
        }

    }





        /* A producer application should continually serve
         * the delivery report queue by calling rd_kafka_poll()
         * at frequent intervals.
         * Either put the poll call in your main loop, or in a
         * dedicated thread, or call it after every
         * rd_kafka_produce() call.
         * Just make sure that rd_kafka_poll() is still called
         * during periods where you are not producing any messages
         * to make sure previously produced messages have their
         * delivery report callback served (and any other callbacks
         * you register). */
        rd_kafka_poll(rk, 0/*non-blocking*/);



    /* Wait for final messages to be delivered or fail.
     * rd_kafka_flush() is an abstraction over rd_kafka_poll() which
     * waits for all messages to be delivered. */
    fprintf(stderr, "%% Flushing final messages..\n");
    rd_kafka_flush(rk, 10*1000 /* wait for max 10 seconds */);

    /* Destroy topic object */
    rd_kafka_topic_destroy(rkt);

    /* Destroy the producer instance */
    rd_kafka_destroy(rk);

    return 0;

 }


