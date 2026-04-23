//
//  sync.cpp
//  
//
//  Created by Кудинова Елизавета on 23.04.2026.
//  Группа 12

#include "../include/common.h"

#ifdef _WIN32
// Windows implementation
void syncInit(SyncPrimitives& sync) {
    sync.readyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    sync.dataAvailableEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    sync.spaceAvailableEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void syncDestroy(SyncPrimitives& sync) {
    CloseHandle(sync.readyEvent);
    CloseHandle(sync.dataAvailableEvent);
    CloseHandle(sync.spaceAvailableEvent);
}

void syncSignalReady(SyncPrimitives& sync) {
    SetEvent(sync.readyEvent);
}

void syncWaitForReady(SyncPrimitives& sync) {
    WaitForSingleObject(sync.readyEvent, INFINITE);
}

void syncSignalDataAvailable(SyncPrimitives& sync) {
    SetEvent(sync.dataAvailableEvent);
}

void syncWaitForDataAvailable(SyncPrimitives& sync) {
    WaitForSingleObject(sync.dataAvailableEvent, INFINITE);
}

void syncSignalSpaceAvailable(SyncPrimitives& sync) {
    SetEvent(sync.spaceAvailableEvent);
}

void syncWaitForSpaceAvailable(SyncPrimitives& sync) {
    WaitForSingleObject(sync.spaceAvailableEvent, INFINITE);
}
#else
// POSIX implementation
void syncInit(SyncPrimitives& sync) {
    pthread_mutex_init(&sync.mutex, NULL);
    pthread_cond_init(&sync.readyCond, NULL);
    pthread_cond_init(&sync.dataAvailableCond, NULL);
    pthread_cond_init(&sync.spaceAvailableCond, NULL);
    sync.readyFlag = 0;
    sync.dataAvailableFlag = 0;
    sync.spaceAvailableFlag = 0;
}

void syncDestroy(SyncPrimitives& sync) {
    pthread_mutex_destroy(&sync.mutex);
    pthread_cond_destroy(&sync.readyCond);
    pthread_cond_destroy(&sync.dataAvailableCond);
    pthread_cond_destroy(&sync.spaceAvailableCond);
}

void syncSignalReady(SyncPrimitives& sync) {
    pthread_mutex_lock(&sync.mutex);
    sync.readyFlag = 1;
    pthread_cond_signal(&sync.readyCond);
    pthread_mutex_unlock(&sync.mutex);
}

void syncWaitForReady(SyncPrimitives& sync) {
    pthread_mutex_lock(&sync.mutex);
    while (sync.readyFlag == 0) {
        pthread_cond_wait(&sync.readyCond, &sync.mutex);
    }
    pthread_mutex_unlock(&sync.mutex);
}

void syncSignalDataAvailable(SyncPrimitives& sync) {
    pthread_mutex_lock(&sync.mutex);
    sync.dataAvailableFlag = 1;
    pthread_cond_signal(&sync.dataAvailableCond);
    pthread_mutex_unlock(&sync.mutex);
}

void syncWaitForDataAvailable(SyncPrimitives& sync) {
    pthread_mutex_lock(&sync.mutex);
    while (sync.dataAvailableFlag == 0) {
        pthread_cond_wait(&sync.dataAvailableCond, &sync.mutex);
    }
    sync.dataAvailableFlag = 0;
    pthread_mutex_unlock(&sync.mutex);
}

void syncSignalSpaceAvailable(SyncPrimitives& sync) {
    pthread_mutex_lock(&sync.mutex);
    sync.spaceAvailableFlag = 1;
    pthread_cond_signal(&sync.spaceAvailableCond);
    pthread_mutex_unlock(&sync.mutex);
}

void syncWaitForSpaceAvailable(SyncPrimitives& sync) {
    pthread_mutex_lock(&sync.mutex);
    while (sync.spaceAvailableFlag == 0) {
        pthread_cond_wait(&sync.spaceAvailableCond, &sync.mutex);
    }
    sync.spaceAvailableFlag = 0;
    pthread_mutex_unlock(&sync.mutex);
}
#endif
