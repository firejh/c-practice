#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/time.h>
#include <errno.h>
#include <getopt.h>

/* Typical include path would be <librdkafka/rdkafka.h>, but this program
 * is builtin from within the librdkafka source tree and thus differs. */
#include <librdkafka/rdkafka.h>  /* for Kafka driver */

static int run = 1;
static rd_kafka_t *rk;
static int exit_eof = 0;
static int wait_eof = 0;  /* number of partitions awaiting EOF */

static enum {
    OUTPUT_HEXDUMP,
    OUTPUT_RAW,
} output = OUTPUT_HEXDUMP;

static void stop (int sig) {
    if (!run) {
        exit(1);
    }

    run = 0;
    fclose(stdin); /* abort fgets() */
}


static void hexdump(FILE *fp, const char *name, const void *ptr, size_t len) {
    const char *p = (const char *)ptr;
    unsigned int of = 0;

    if (name) {
        fprintf(fp, "%s hexdump (%zd bytes):\n", name, len);
    }

    for (of = 0 ; of < len ; of += 16) {
        char hexen[16*3+1];
        char charen[16+1];
        int hof = 0;

        int cof = 0;
        int i;

        for (i = of ; i < (int)of + 16 && i < (int)len ; i++) {
            hof += sprintf(hexen+hof, "%02x ", p[i] & 0xff);
            cof += sprintf(charen+cof, "%c", isprint((int)p[i]) ? p[i] : '.');
        }
        fprintf(fp, "%08x: %-48s %-16s\n", of, hexen, charen);
    }
}

/**
 * Kafka logger callback (optional)
 */
static void logger(const rd_kafka_t *rk, int level, const char *fac, const char *buf) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    fprintf(stdout, "%u.%03u RDKAFKA-%i-%s: %s: %s\n",
        (int)tv.tv_sec, (int)(tv.tv_usec / 1000), level, fac, rd_kafka_name(rk), buf);
}



/**
 * Handle and print a consumed message.
 * Internally crafted messages are also used to propagate state from
 * librdkafka to the application. The application needs to check
 * the `rkmessage->err` field for this purpose.
 */
static void msg_consume (rd_kafka_message_t *rkmessage, void *opaque) {
    if (rkmessage->err) {
        if (rkmessage->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
            fprintf(stderr, "%% Consumer reached end of %s [%"PRId32"] message queue at offset %"PRId64"\n",
                    rd_kafka_topic_name(rkmessage->rkt),
                    rkmessage->partition, rkmessage->offset);

            if (exit_eof && --wait_eof == 0) {
                fprintf(stderr, "%% All partition(s) reached EOF: exiting\n");
                run = 0;
            }

            return;
        }

        if (rkmessage->rkt) {
            fprintf(stderr, "%% Consume error for topic \"%s\" [%"PRId32"] offset %"PRId64": %s\n",
                    rd_kafka_topic_name(rkmessage->rkt),
                    rkmessage->partition,
                    rkmessage->offset,
                    rd_kafka_message_errstr(rkmessage));
        } else {
            fprintf(stderr, "%% Consumer error: %s: %s\n",
                            rd_kafka_err2str(rkmessage->err),
                            rd_kafka_message_errstr(rkmessage));
        }

        if (rkmessage->err == RD_KAFKA_RESP_ERR__UNKNOWN_PARTITION ||
                rkmessage->err == RD_KAFKA_RESP_ERR__UNKNOWN_TOPIC) {
            run = 0;
        }

        return;
    }

    fprintf(stdout, "%% Message (topic %s [%"PRId32"], offset %"PRId64", %zd bytes):\n",
            rd_kafka_topic_name(rkmessage->rkt),
            rkmessage->partition,
            rkmessage->offset,
            rkmessage->len);

    if (rkmessage->key_len) {
        if (output == OUTPUT_HEXDUMP) {
            hexdump(stdout, "Message Key",
                            rkmessage->key, rkmessage->key_len);
        } else {
            printf("Key: %.*s\n",
                         (int) rkmessage->key_len, (char *) rkmessage->key);
        }
    }

    if (output == OUTPUT_HEXDUMP)
        hexdump(stdout, "Message Payload",
                        rkmessage->payload, rkmessage->len);
    else
        printf("%.*s\n",
                     (int)rkmessage->len, (char *)rkmessage->payload);
}

static void print_partition_list (FILE *fp,
                                                                    const rd_kafka_topic_partition_list_t
                                                                    *partitions) {
    int i;
    for (i = 0 ; i < partitions->cnt ; i++) {
        fprintf(stderr, "%s %s [%"PRId32"] offset %"PRId64,
                i > 0 ? ",":"",
                partitions->elems[i].topic,
                partitions->elems[i].partition,
                partitions->elems[i].offset);
    }
    fprintf(stderr, "\n");

}

static void rebalance_cb (rd_kafka_t *rk,
                                                    rd_kafka_resp_err_t err,
                                                    rd_kafka_topic_partition_list_t *partitions,
                                                    void *opaque) {

    fprintf(stderr, "%% Consumer group rebalanced: ");

    switch (err)
    {
        case RD_KAFKA_RESP_ERR__ASSIGN_PARTITIONS:
            fprintf(stderr, "assigned:\n");
            print_partition_list(stderr, partitions);
            rd_kafka_assign(rk, partitions);
            wait_eof += partitions->cnt;
            break;

        case RD_KAFKA_RESP_ERR__REVOKE_PARTITIONS:
            fprintf(stderr, "revoked:\n");
            print_partition_list(stderr, partitions);
            rd_kafka_assign(rk, NULL);
            wait_eof = 0;
            break;

        default:
            fprintf(stderr, "failed: %s\n",
                            rd_kafka_err2str(err));
            rd_kafka_assign(rk, NULL);
            break;
    }
}

static void sig_usr1 (int sig) {
    rd_kafka_dump(stdout, rk);
}

int main (int argc, char **argv) {
  char *brokers = "192.168.100.138:9092";
  rd_kafka_conf_t *conf;
  rd_kafka_topic_conf_t *topic_conf;
  char errstr[512];
  char tmp[16];

  rd_kafka_resp_err_t err;
  char *group = NULL;
  rd_kafka_topic_partition_list_t *topics;

  /* Kafka configuration */
  conf = rd_kafka_conf_new();

  /* Set logger */
  rd_kafka_conf_set_log_cb(conf, logger);

    /* Quick termination */
  // 查看configure文档,这个参数的意思是rdkafka_destroy函数退出时需要回收资源,这个选项会跳过这个步骤以快速退出
    snprintf(tmp, sizeof(tmp), "%i", SIGIO);
    rd_kafka_conf_set(conf, "internal.termination.signal", tmp, NULL, 0);

    /* Topic configuration */
    topic_conf = rd_kafka_topic_conf_new();

    signal(SIGINT, stop);
    signal(SIGUSR1, sig_usr1);

  /*
  * Client/Consumer group
  */

  /* Consumer groups require a group id */
  group = "rdkafka_consumer_example";
  if (rd_kafka_conf_set(conf, "group.id", group, errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
          fprintf(stderr, "%% %s\n", errstr);
          exit(1);
  }

  /* Consumer groups always use broker based offset storage */
  if (rd_kafka_topic_conf_set(topic_conf, "offset.store.method", "broker", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
          fprintf(stderr, "%% %s\n", errstr);
          exit(1);
  }

  /* Set default topic config for pattern-matched topics. */
  rd_kafka_conf_set_default_topic_conf(conf, topic_conf);

  /* Callback called on partition assignment changes */
  rd_kafka_conf_set_rebalance_cb(conf, rebalance_cb);

  /* Create Kafka handle */
  if (!(rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr)))) {
      fprintf(stderr,
              "%% Failed to create new consumer: %s\n",
              errstr);
      exit(1);
  }

  /* Add brokers */
  if (rd_kafka_brokers_add(rk, brokers) == 0) {
    fprintf(stderr, "%% No valid brokers specified\n");
    exit(1);
  }

  /* Redirect rd_kafka_poll() to consumer_poll() */
  rd_kafka_poll_set_consumer(rk);

  topics = rd_kafka_topic_partition_list_new(1);

  char *topic = "test5";
  int32_t partition = -1;
  rd_kafka_topic_partition_list_add(topics, topic, partition);

  fprintf(stderr, "%% Subscribing to %d topics\n", topics->cnt);
  if ((err = rd_kafka_subscribe(rk, topics))) {
    fprintf(stderr, "%% Failed to start consuming topics: %s\n",
            rd_kafka_err2str(err));
    exit(1);
  }

//  fprintf(stderr, "%% Assigning %d partitions\n", topics->cnt);
//
//  if ((err = rd_kafka_assign(rk, topics))) {
//    fprintf(stderr,
//            "%% Failed to assign partitions: %s\n",
//            rd_kafka_err2str(err));
//  }

  while (run) {
    rd_kafka_message_t *rkmessage;

    rkmessage = rd_kafka_consumer_poll(rk, 1000);
    if (rkmessage) {
      msg_consume(rkmessage, NULL);
      rd_kafka_message_destroy(rkmessage);
    }
  }

    err = rd_kafka_consumer_close(rk);
    if (err) {
        fprintf(stderr, "%% Failed to close consumer: %s\n", rd_kafka_err2str(err));
    } else {
        fprintf(stderr, "%% Consumer closed\n");
    }

  rd_kafka_topic_partition_list_destroy(topics);

  /* Destroy handle */
  rd_kafka_destroy(rk);

    /* Let background threads clean up and terminate cleanly. */
    run = 5;
    while (run-- > 0 && rd_kafka_wait_destroyed(1000) == -1) {
        printf("Waiting for librdkafka to decommission\n");
    }
    if (run <= 0) {
        rd_kafka_dump(stdout, rk);
    }

    return 0;
}
