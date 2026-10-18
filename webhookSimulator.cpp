/*
When a payment succeeds, Stripe notifies the merchant by sending a webhook, an HTTP call to the
 merchant’s server. That server is called an endpoint (e.g. ep1). If the endpoint is down, Stripe tries again later.

For this multi-part question, we will simulate that delivery system. We won’t send anything over the network. 
You will implement a function simulate that returns a log of every delivery attempt. 
The implementation is broken down into five parts.
Terms used throughout

event: one notification to deliver, identified by a unique eventId, to one endpoint, arriving at a given time. 
The position of an event in the events array is its arrival order (0, 1, 2, ...). It is used to break ties.
endpoint: a merchant’s server that receives webhooks, identified by endpointId. Different events can go to the same endpoint.
attempt: one try at delivering an event. The first try is attempt number 1. An attempt either succeeds or fails.
 It fails if its endpoint is inside an outage at that moment (see outages below) and succeeds otherwise. time is measured in integer seconds.

Function Description
Complete the function simulate in the editor below.

Function Parameters

int maxAttempts: the maximum total number of attempts per event (used from Part 2).
int retryDelay: the base delay in seconds before a retry (used from Part 2).
int K: the number of consecutive failed attempts that disables an endpoint (used in Part 4).
string outages[m]: lines with the schema endpointId,start,end, meaning that endpoint is down for times start <= t < end.
An endpoint can have several outage lines. An attempt to an endpoint at time t fails if t is inside any of its outages.


string events[n]: lines with the schema timestamp,eventId,endpointId, sorted by timestamp. 
Example: 5,e2,ep1 means event e2 arrives at second 5 and must be delivered to ep1.

Returns
string[]: the log of everything that happened, in chronological order. Each line has one of two schemas:

time,eventId,attemptNumber,SUCCESS or time,eventId,attemptNumber,FAIL: an attempt was made.
time,eventId,-,DROPPED: the event was given up on without an attempt (Part 4 onward).

Lines with the same time are ordered by the event’s arrival order (smaller first).

Part 1: Single attempt (test cases 1-4)
Each event is attempted exactly once, at its arrival time.

Example: outages = ["ep1,0,8"]

events:  0,e1,ep1    5,e2,ep2    9,e3,ep1
output:  0,e1,1,FAIL
         5,e2,1,SUCCESS
         9,e3,1,SUCCESS

Part 2: Fixed retries (test cases 5-8)
If an attempt fails, the event is retried retryDelay seconds after the failed attempt, up to maxAttempts total attempts.
 Stop at the first success. If the last allowed attempt fails, the event is abandoned and nothing more is logged. 
 Retries interleave chronologically with newly arriving events.

Example 1: maxAttempts = 3, retryDelay = 5, outages = ["ep1,0,8"], event 0,evt1,ep1

0,evt1,1,FAIL
5,evt1,2,FAIL
10,evt1,3,SUCCESS

Example 2: maxAttempts = 2, retryDelay = 5, outages = ["ep1,0,8"], events 0,e1,ep1 and 5,e2,ep1

0,e1,1,FAIL
5,e1,2,FAIL       (e1 abandoned: 2 attempts used)
5,e2,1,FAIL       (same time: e1 has the smaller arrival order)
10,e2,2,SUCCESS

Part 3: Exponential backoff (test cases 9-11)
Retrying at a fixed interval can overload a struggling server, so the delay now doubles. 
The delay after failed attempt number k is retryDelay * 2^(k-1): retryDelay,
then 2 * retryDelay, then 4 * retryDelay, and so on, measured from the time 
of the attempt that just failed.

Example: maxAttempts = 4, retryDelay = 5, outages = ["ep1,0,20"], event 0,e1,ep1

0,e1,1,FAIL
5,e1,2,FAIL       (delay 5)
15,e1,3,FAIL      (delay 10)
35,e1,4,SUCCESS   (delay 20)

Part 4: Automatic endpoint disabling (test cases 12-14)
If an endpoint keeps failing, Stripe stops trying. Count consecutive failed attempts per endpoint, across all of its events, in log order. 
A success resets that endpoint’s counter to 0. When the counter reaches K, the endpoint is disabled for the rest of the simulation, 
immediately after that failing attempt is logged:

Every event of that endpoint still waiting for an attempt (the event whose failure triggered the disable, and any others with a retry pending) is logged as time,eventId,-,DROPPED at the time of disabling, in arrival order. They are never attempted again.
Any later event arriving for a disabled endpoint is logged as time,eventId,-,DROPPED at its arrival time and is never attempted.

Example: maxAttempts = 5, retryDelay = 1, K = 3, outages = ["ep1,0,100"], events 0,e1,ep1, 0,e2,ep1, 50,e3,ep1

0,e1,1,FAIL       (consecutive failures for ep1: 1)
0,e2,1,FAIL       (2)
1,e1,2,FAIL       (3, so ep1 is disabled)
1,e1,-,DROPPED    (e1 was waiting for a retry)
1,e2,-,DROPPED    (e2's retry was due at 1 but ep1 is now disabled)
50,e3,-,DROPPED   (arrives at a disabled endpoint)

Part 5: Rate cap per endpoint (test cases 15-17)
A struggling server shouldn’t be flooded either. An endpoint can receive at most one attempt per second. At each integer second, look at all attempts that are due for an endpoint (new events and retries, including any left over from earlier seconds). Only the one whose event has the smallest arrival order is made. The others stay due and are considered again at the next second. Details:

Attempt numbers don’t change when an attempt is delayed.
The backoff for later retries is computed from the actual time of the attempt, not the originally due time.
Different endpoints are independent. The cap applies per endpoint.
If an endpoint gets disabled while attempts are waiting, they are dropped as in Part 4.

Example: maxAttempts = 2, retryDelay = 5, K = 100, outages = ["ep1,0,1"], events 0,e1,ep1, 0,e2,ep1, 0,e3,ep1

0,e1,1,FAIL       (e2 and e3 are also due at 0, but only one attempt per second)
1,e2,1,SUCCESS    (outage over at t=1; e3 still waiting)
2,e3,1,SUCCESS
5,e1,2,SUCCESS    (retry: 0 + 5)

Input Format for Custom Testing
Input from stdin is processed as follows and passed to the function.

The first line contains the integer maxAttempts.
The second line contains the integer retryDelay.
The third line contains the integer K.
The fourth line contains m, the size of outages, followed by m lines.
The next line contains n, the size of events, followed by n lines.
*/

#include<bits/stdc++.h>
using namespace std;

vector<string> simulate(int maxAttempts, int retryDelay, int K, vector<string> outages, vector<string> events){
    map<string,vector<pair<int,int>>> outageMap;
    for(int i = 0; i < outages.size(); i++){
        stringstream ss(outages[i]);
        string endpoint, start, end;
        getline(ss, endpoint, ',');
        getline(ss, start, ',');
        getline(ss, end, ',');
        outageMap[endpoint].push_back({stoi(start), stoi(end)});
    }
    vector<string> logger;
    map<string,int> failAttemptMap;
    map<string,int> lastAttemptTime;
    map<int,int> arrivalTimeMap;
    set<string> disabledEndpoints;
    set<tuple<int,int,string,string,int>> st;
    for(int i = 0; i < events.size(); i++){
        stringstream ss(events[i]);
        string timestamp, eventId, endpointId;
        getline(ss, timestamp, ',');
        getline(ss, eventId, ',');
        getline(ss, endpointId, ',');
        int arrivalTime = stoi(timestamp);
        arrivalTimeMap[i] = arrivalTime;
        st.insert({arrivalTime, i, eventId, endpointId, 1});
    }

    while(!st.empty()){
        auto it = st.begin();
        int currentTime = get<0>(*it);
        int arrivalIndex = get<1>(*it);
        string eventId = get<2>(*it);
        string endpointId = get<3>(*it);
        int attemptNumber = get<4>(*it);
        st.erase(it);
        if(disabledEndpoints.count(endpointId)){
            logger.push_back(to_string(currentTime) + "," +
                             eventId + ",-,DROPPED");
            continue;
        }
        if(lastAttemptTime.count(endpointId) && lastAttemptTime[endpointId] == currentTime){
            st.insert({currentTime + 1, arrivalIndex,eventId, endpointId, attemptNumber});
            continue;
        }
        lastAttemptTime[endpointId] = currentTime;
        bool failed = false;
        if(outageMap.count(endpointId)){
            for(auto &outage : outageMap[endpointId]){
                if(outage.first <= currentTime &&
                   currentTime < outage.second){
                    failed = true;
                    break;
                }
            }
        }
        if(failed){
            logger.push_back(to_string(currentTime) + "," +eventId + "," + to_string(attemptNumber) + ",FAIL");
            failAttemptMap[endpointId]++;
            if(failAttemptMap[endpointId] >= K){ disabledEndpoints.insert(endpointId);
                vector<pair<int,string>> droppedEvents;
                droppedEvents.push_back({arrivalIndex, eventId});
                for(auto iter = st.begin(); iter != st.end(); ){
                    int idx = get<1>(*iter);
                    string pendingEventId = get<2>(*iter);
                    string pendingEndpointId = get<3>(*iter);
                    if(pendingEndpointId == endpointId &&
                       arrivalTimeMap[idx] <= currentTime){
                        droppedEvents.push_back({idx, pendingEventId});
                        iter = st.erase(iter);
                    }
                    else{
                        ++iter;
                    }
                }
                sort(droppedEvents.begin(), droppedEvents.end());
                for(auto &dropped : droppedEvents){
                    logger.push_back(to_string(currentTime) + "," + dropped.second + ",-,DROPPED");
                }
                continue;
            }

            
            if(attemptNumber < maxAttempts){
                int delay = retryDelay * (int)pow(2, attemptNumber - 1);
                st.insert({currentTime + delay, arrivalIndex, eventId, endpointId, attemptNumber + 1});
            }
        }
        else{
            logger.push_back(to_string(currentTime) + "," +eventId + "," +to_string(attemptNumber) + ",SUCCESS");
            failAttemptMap[endpointId] = 0;
        }
    }

    return logger;
}