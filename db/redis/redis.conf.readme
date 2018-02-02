################################# REPLICATION #################################
# Master-Slave replication. Use slaveof to make a Redis instance a copy of                             
# another Redis server. A few things to understand ASAP about Redis replication.                       
关于redis复制，有以下几个需要注意的点
1) 可以通过设置参数，规定当slave与master链接少于指定数目时候，停止接受写请求；

# 下面这个参数指定了主从有效连接的最少数目                                               
# min-slaves-to-write 3       
# 下面这个参数指定了当主从的ping包延迟少于其参数值的时候，
# 就认为slave处于online状态，通常slave每1秒向master发送一次心跳包(ping)                                                                     
# min-slaves-max-lag 10                                                                             
#                                                                                                   
# Setting one or the other to 0 disables the feature.                                               
#                                                                                                   
# By default min-slaves-to-write is set to 0 (feature disabled) and                                 
# min-slaves-max-lag is set to 10. 

2) 当slave与master之间的数据差异少于某个size的时候，二者之间进行增量同步；
                                                                                    
# 上面第二点提到的master与slave之间的数据差异就是下面这个参数，这个参数指定了
# master和slave之间的backlog buffer的size，这个backlog buffer在redis启动时就被
# 创建，其大小不会被修改。360公司在主从位于不同机房的时候把它设为为2G。                                                                                                  
# repl-backlog-size 1mb 
                                                  
# 当主从之间的连接断开持续时间超过下面指定的参数后，backlog buffer会被释放掉。
# 指定为0意味着backlog永不释放。                                                                                                  
# repl-backlog-ttl 3600



# 当客户端由于某些原因(如Pub/Sub模式下客户端的处理速度慢于发送端制造数据的速度)
# 不能及时从server端接收数据时，server端可以使用输出buffer来缓存其“断线”或者
# “繁忙”期间累积的数据。                                                                                   
# 客户端的种类有以下三种：                                                                             
# normal -> normal clients including MONITOR clients                     正常客户端                                                 
# slave  -> slave clients                                                从客户端                                                 
# pubsub -> clients subscribed to at least one pubsub channel or pattern pubsub客户端                              
#                                                                                                      
# client-output-buffer-limit指令的格式如下:                                                             
# client-output-buffer-limit <class> <hard limit> <soft limit> <soft seconds>                          
#                                                                                                      
# 一个客户端如果达到硬限制，或者在一段时间内持续达到软限制，则会被系统强制下线。                       
# 例如系统的硬限制是32M，其软限制则是10秒内连续超过16M。如果一个客户端的输出buffer
# 达到了32M或者连续10s都超过16M，则客户端会被下线。                                                    
#                                                                                                      
# 正常阻塞模式下客户端是不会受这些参数影响的，因为它们与server端之间是asking-answer模式，
# 当客户端发送的请求是异步请求的时候，其接收的参数可能会产生累积。                                     
#                                                                                                      
# pubsub和slave客户端的请求都是异步请求，所以会用到这个指令。  
# Both the hard or the soft limit can be disabled by setting them to zero.                             
client-output-buffer-limit normal 0 0 0                                                                
client-output-buffer-limit slave 256mb 64mb 60                                                         
client-output-buffer-limit pubsub 32mb 8mb 60  

############################# EVENT NOTIFICATION ##############################   
# Redis允许client监控某些key上发生的某些操作，当这些操作发生的时候，client会收到
# Redis以pub/sub模式发送来的notify。                                                                
# 例如一个客户端监控db 0上发生的对key "foo"的del操作，当这个操作发生时会有如下通知:
# PUBLISH __keyspace@0__:foo del                                                                    
# PUBLISH __keyevent@0__:del foo                                                                    
#                                                                                                   
# 客户端可以设置相关参数(单个字符)来订阅一类事件：                                                  
#  K     Keyspace events, published with __keyspace@<db>__ prefix.                                  
#  E     Keyevent events, published with __keyevent@<db>__ prefix.                                  
#  g     Generic commands (non-type specific) like DEL, EXPIRE, RENAME, ...                         
#  $     String commands                                                                            
#  l     List commands                                                                              
#  s     Set commands                                                                               
#  h     Hash commands                                                                              
#  z     Sorted set commands                                                                        
#  x     Expired events (events generated every time a key expires)                                 
#  e     Evicted events (events generated when a key is evicted for maxmemory)                      
#  A     Alias for g$lshzxe, so that the "AKE" string means all the events.                         
#                                                                                                   
#  Example: 假设要监控list和一般的事件，可以配置如下指令：                                          
#  notify-keyspace-events Elg                                                                       
#                                                                                                   
#  Example 2: 想要监控过期数据事件并收到__keyevent@0__:expired类似的event通知，
#  可以用如下指令：                                       
#  notify-keyspace-events Ex                                                                        
#                                                                                                   
# 默认情况下这个指令是不会被启用的(指令参数为空)，因为它比较耗费资源。
# 另外指令参数中一定要有K或者E，否则什么通知都收不到。
notify-keyspace-events "" 
