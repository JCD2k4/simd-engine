#include <cstdio>
#include <vector>
#include <iostream>
#include "parquet_loader.hpp"
#include "benchmark/benchmark.h"

/*
VendorID
tpep_pickup_datetime
tpep_dropoff_datetime
passenger_count
trip_distance
RatecodeID
store_and_fwd_flag
PULocationID
DOLocationID
payment_type
fare_amount
extra
mta_tax
tip_amount
tolls_amount
improvement_surcharge
total_amount
congestion_surcharge
Airport_fee
cbd_congestion_fee
*/


const char* FILE_PATH = "../yellow_tripdata_2026-01.parquet";

void load_file();
float calculate_column_average(const char* column_name);

template <typename T>
T calculate_column_total(const char* column_name);

int main() {
    //load_file();
    float total_trip_distance = calculate_column_total<float>("trip_distance");
    float trip_avg = calculate_column_average("trip_distance");
    std::cout << total_trip_distance << "\n";
    std::cout << trip_avg << "\n";
    return 0;
}

// PHASE 1

template <typename T>
T calculate_column_total(const char* column_name){

    // This function is mainly for just simply adding columns, where the data is added 1 by 1
    std::vector<T> dist = pql::load_column<T>(FILE_PATH, column_name);
    
    T total = 0;
    for (auto it = dist.cbegin(); it != dist.cend(); ++it){
        total += *it;
    }

    return total;

}
float calculate_column_average(const char* column_name){
    // This function is mainly for just simply finding the average of columns, where the data is added 1 by 1 then finally divided
    std::vector<float> dist = pql::load_column<float>(FILE_PATH, column_name);
    float total = 0;
    int count = 0;
    for (auto it = dist.cbegin(); it != dist.cend(); ++it){
        total += *it;
        count++;
    }

    return total / count;

}

float get_column_min(const char* column_name){

    std::vector<float> dist = pql::load_column<float>(FILE_PATH, column_name);
    float smallest = 0;
    for (auto it = dist.cbegin(); it != dist.cend(); ++it){
        smallest = (*it < smallest) ? *it : smallest;
    }

    return smallest;
}

float get_column_max(const char* column_name){
    std::vector<float> dist = pql::load_column<float>(FILE_PATH, column_name);
    float largest = 0;
    for (auto it = dist.cbegin(); it != dist.cend(); ++it){
        largest = (*it > largest) ? *it : largest;
    }

    return largest;
}

// PHASE 2




void load_file(){

    std::vector<float> dist = pql::load_float_column(FILE_PATH, "trip_distance");

    // dist.data() is a plain float* over 3,724,889 contiguous values.
    for (size_t i = 0; i < dist.size(); ++i) dist[i] = dist[i] * dist[i];

    for (const std::string& name : pql::list_columns("../yellow_tripdata_2026-01.parquet")) {
        printf("%s\n", name.c_str());
    }

    printf("%zu values, first = %.4f\n", dist.size(), dist[0]);
}