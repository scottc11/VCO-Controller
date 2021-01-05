
### MBED Version 5.14.1
- tag = mbed-os-5.14.1

Cloning project

```
git clone https://github.com/scottc11/inversion.git
git submodule init
git submodule update
```

pull latest mbed-lib changes
```
git submodule update --remote
```

---
# [MBED RTOS](https://os.mbed.com/docs/mbed-os/v5.14/apis/rtos.html)
Thread: The class that allows defining, creating and controlling parallel tasks.
ThisThread: The class with which you can control the current thread.
Mutex: The class used to synchronize the execution of threads.
Semaphore: The class that manages thread access to a pool of shared resources of a certain type.
Queue: The class that allows you to queue pointers to data from producer threads to consumer threads.
EventQueue: The class that provides a flexible queue for scheduling events. You can use the EventQueue class for synchronization between multiple threads, or to move events out of interrupt context
UserAllocatedEvent: The class that provides APIs to create and configure static events
MemoryPool: This class that you can use to define and manage fixed-size memory pools
Mail: The API that provides a queue combined with a memory pool for allocating messages.
EventFlags: An event channel that provides a generic way of notifying other threads about conditions or events. You can call some EventFlags functions from ISR context, and each EventFlags object can support up to 31 flags.
Event: The queue to store events, extract them and execute them later.
ConditionVariable: The ConditionVariable class provides a mechanism to safely wait for or signal a single state change. You cannot call ConditionVariable functions from ISR context.
Kernel: Kernel namespace implements functions to control or read RTOS information, such as tick count.
---
# [MBED Thread Safe Drivers and Methods](https://os.mbed.com/docs/mbed-os/v6.6/apis/thread-safety.html)

# [mbed_app.json](https://os.mbed.com/docs/mbed-os/v5.11/reference/configuration.html)

This file is used to override the default MBED build configurations


#### [Possible implementation of HAL framework in MBED](https://os.mbed.com/forum/platform-34-ST-Nucleo-F401RE-community/topic/4963/?page=2)

#### [another](https://os.mbed.com/users/gregeric/code/Nucleo_Hello_Encoder/docs/tip/main_8cpp_source.html)