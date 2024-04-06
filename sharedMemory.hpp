// Shared Memory

struct table {
    int data[2];
    int in;
    int out;
};

static const int SIZE = sizeof(table);
static const int bufferSize = 2;
static const char* NAME = "/producer_consumer_jlyogky"; // Name of shared memory
static const int totalProduction = 15; // Total amount that will be produced
