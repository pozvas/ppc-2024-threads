// Copyright 2024 Savchuk Anton
#include "omp/savchuk_a_crs_matmult_omp/include/crs_matmult_omp.hpp"

bool SavchukCRSMatMultOMPSequential::validation() {
  internal_order_test();

  return taskData->inputs_count[1] == taskData->inputs_count[2] &&
         taskData->outputs_count[0] == taskData->inputs_count[0] &&
         taskData->outputs_count[1] == taskData->inputs_count[3];
}

bool SavchukCRSMatMultOMPSequential::pre_processing() {
  internal_order_test();

  auto* matrix1 = reinterpret_cast<double*>(taskData->inputs[0]);
  auto* matrix2 = reinterpret_cast<double*>(taskData->inputs[1]);
  numRows1 = taskData->inputs_count[0];
  numCols1 = taskData->inputs_count[1];
  numRows2 = taskData->inputs_count[2];
  numCols2 = taskData->inputs_count[3];

  result = new double[numRows1 * numCols2]{0.0};

  int notNullNumbers = 0;
  for (int i = 0; i < numRows1; ++i) {
    rowPtr1.push_back(notNullNumbers);
    for (int j = 0; j < numCols1; ++j) {
      int index = i * numCols1 + j;
      if (matrix1[index] != 0.0) {
        values1.push_back(matrix1[index]);
        colPtr1.push_back(j);
        notNullNumbers++;
      }
    }
  }
  rowPtr1.push_back(notNullNumbers);

  notNullNumbers = 0;
  for (int i = 0; i < numRows2; ++i) {
    rowPtr2.push_back(notNullNumbers);
    for (int j = 0; j < numCols2; ++j) {
      int index = i * numCols2 + j;
      if (matrix2[index] != 0.0) {
        values2.push_back(matrix2[index]);
        colPtr2.push_back(j);
        notNullNumbers++;
      }
    }
  }
  rowPtr2.push_back(notNullNumbers);

  return true;
}

bool SavchukCRSMatMultOMPSequential::run() {
  internal_order_test();

  for (int i = 0; i < numRows1; i++) {
    for (int j = rowPtr1[i]; j < rowPtr1[i + 1]; j++) {
      int row1 = i;
      int col1 = colPtr1[j];
      double val1 = values1[j];
      for (int k = rowPtr2[col1]; k < rowPtr2[col1 + 1]; k++) {
        int col2 = colPtr2[k];
        double val2 = values2[k];
        int index = row1 * numCols2 + col2;
        result[index] += val1 * val2;
      }
    }
  }

  return true;
}

bool SavchukCRSMatMultOMPSequential::post_processing() {
  internal_order_test();

  auto* out_ptr = reinterpret_cast<double*>(taskData->outputs[0]);
  for (int i = 0; i < numRows1 * numCols2; i++) {
    out_ptr[i] = result[i];
  }

  delete[] result;

  return true;
}

bool SavchukCRSMatMultOMPParallel::validation() {
  internal_order_test();

  return taskData->inputs_count[1] == taskData->inputs_count[2] &&
         taskData->outputs_count[0] == taskData->inputs_count[0] &&
         taskData->outputs_count[1] == taskData->inputs_count[3];
}

bool SavchukCRSMatMultOMPParallel::pre_processing() {
  internal_order_test();

  auto* matrix1 = reinterpret_cast<double*>(taskData->inputs[0]);
  auto* matrix2 = reinterpret_cast<double*>(taskData->inputs[1]);
  numRows1 = taskData->inputs_count[0];
  numCols1 = taskData->inputs_count[1];
  numRows2 = taskData->inputs_count[2];
  numCols2 = taskData->inputs_count[3];

  result = new double[numRows1 * numCols2]{0.0};

  int notNullNumbers = 0;
  for (int i = 0; i < numRows1; ++i) {
    rowPtr1.push_back(notNullNumbers);
    for (int j = 0; j < numCols1; ++j) {
      int index = i * numCols1 + j;
      if (matrix1[index] != 0.0) {
        values1.push_back(matrix1[index]);
        colPtr1.push_back(j);
        notNullNumbers++;
      }
    }
  }
  rowPtr1.push_back(notNullNumbers);

  notNullNumbers = 0;
  for (int i = 0; i < numRows2; ++i) {
    rowPtr2.push_back(notNullNumbers);
    for (int j = 0; j < numCols2; ++j) {
      int index = i * numCols2 + j;
      if (matrix2[index] != 0.0) {
        values2.push_back(matrix2[index]);
        colPtr2.push_back(j);
        notNullNumbers++;
      }
    }
  }
  rowPtr2.push_back(notNullNumbers);

  return true;
}

bool SavchukCRSMatMultOMPParallel::run() {
  internal_order_test();

#pragma omp parallel for
  for (int i = 0; i < numRows1; i++) {
    for (int j = rowPtr1[i]; j < rowPtr1[i + 1]; j++) {
      int row1 = i;
      int col1 = colPtr1[j];
      double val1 = values1[j];
      for (int k = rowPtr2[col1]; k < rowPtr2[col1 + 1]; k++) {
        int col2 = colPtr2[k];
        double val2 = values2[k];
        int index = row1 * numCols2 + col2;
#pragma omp atomic
        result[index] += val1 * val2;
      }
    }
  }

  return true;
}

bool SavchukCRSMatMultOMPParallel::post_processing() {
  internal_order_test();

  auto* out_ptr = reinterpret_cast<double*>(taskData->outputs[0]);
  for (int i = 0; i < numRows1 * numCols2; i++) {
    out_ptr[i] = result[i];
  }

  delete[] result;

  return true;
}