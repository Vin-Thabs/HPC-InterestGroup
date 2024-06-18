#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

// Function to calculate mean
double calculateMean(const vector<double>& data) {
    double sum = 0.0;
    for (double value : data) {
        sum += value;
    }
    return sum / data.size();
}

// Function to calculate sample variance
double calculateSampleVariance(const vector<double>& data, double mean) {
    double sum = 0.0;
    for (double value : data) {
        sum += (value - mean) * (value - mean);
    }
    return sum / (data.size() - 1);
}

// Function to calculate covariance
double calculateCovariance(const vector<double>& data1, const vector<double>& data2, double mean1, double mean2) {
    double sum = 0.0;
    for (size_t i = 0; i < data1.size(); ++i) {
        sum += (data1[i] - mean1) * (data2[i] - mean2);
    }
    return sum / (data1.size() - 1);
}

int main() {
    // Example data
    vector<double> data1 = {0.69, -1.31, 0.39, 0.09, 1.29, 0.49, 0.19, -0.81, -0.31, -0.71};
    vector<double> data2 = {0.49, -1.21, 0.99, 0.29, 1.09, 0.79, -0.31, -0.31, -0.81, -1.01};

    // Calculate means
    double mean1 = calculateMean(data1);
    double mean2 = calculateMean(data2);

    // Calculate sample variances
    double variance1 = calculateSampleVariance(data1, mean1);
    double variance2 = calculateSampleVariance(data2, mean2);

    // Calculate covariance
    double covariance = calculateCovariance(data1, data2, mean1, mean2);

    // Output results
    cout << "Mean of data1: " << mean1 << endl;
    cout << "Mean of data2: " << mean2 << endl;
    cout << "Sample Variance of data1: " << variance1 << endl;
    cout << "Sample Variance of data2: " << variance2 << endl;
    cout << "Covariance: " << covariance << endl;

    return 0;
}
