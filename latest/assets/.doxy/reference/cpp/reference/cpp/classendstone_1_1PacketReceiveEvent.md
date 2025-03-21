

# Class endstone::PacketReceiveEvent



[**ClassList**](annotated.md) **>** [**endstone**](namespaceendstone.md) **>** [**PacketReceiveEvent**](classendstone_1_1PacketReceiveEvent.md)



_Called when the server receives a packet from a connected client._ 

* `#include <endstone/event/server/packet_receive_event.h>`



Inherits the following classes: [endstone::Cancellable](classendstone_1_1Cancellable.md)
































## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  [**const**](classendstone_1_1Vector.md) std::string | [**NAME**](#variable-name)   = `"PacketReceiveEvent"`<br> |










































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**PacketReceiveEvent**](#function-packetreceiveevent) ([**Player**](classendstone_1_1Player.md) & player, [**int**](classendstone_1_1Vector.md) packet\_id, std::string\_view payload) <br> |
| virtual std::string | [**getEventName**](#function-geteventname) () override const<br> |
|  [**int**](classendstone_1_1Vector.md) | [**getPacketId**](#function-getpacketid) () const<br>_Gets the ID of the packet._  |
|  std::string\_view | [**getPayload**](#function-getpayload) () const<br>_Gets the raw packet data_ **excluding** _the header._ |
|  [**Player**](classendstone_1_1Player.md) & | [**getPlayer**](#function-getplayer) () const<br> |


## Public Functions inherited from endstone::Cancellable

See [endstone::Cancellable](classendstone_1_1Cancellable.md)

| Type | Name |
| ---: | :--- |
| virtual [**void**](classendstone_1_1Vector.md) | [**cancel**](classendstone_1_1Cancellable.md#function-cancel) () <br>_Cancel this event. A cancelled event will not be executed in the server, but will still pass to other plugins._  |
| virtual [**bool**](classendstone_1_1Vector.md) | [**isCancelled**](classendstone_1_1Cancellable.md#function-iscancelled) () const<br>_Gets the cancellation state of this event. A cancelled event will not be executed in the server, but will still pass to other plugins._  |
| virtual [**void**](classendstone_1_1Vector.md) | [**setCancelled**](classendstone_1_1Cancellable.md#function-setcancelled) ([**bool**](classendstone_1_1Vector.md) cancel) <br>_Sets the cancellation state of this event. A cancelled event will not be executed in the server, but will still pass to other plugins._  |


## Public Functions inherited from endstone::ICancellable

See [endstone::ICancellable](classendstone_1_1ICancellable.md)

| Type | Name |
| ---: | :--- |
| virtual [**void**](classendstone_1_1Vector.md) | [**cancel**](classendstone_1_1ICancellable.md#function-cancel) () = 0<br> |
| virtual [**bool**](classendstone_1_1Vector.md) | [**isCancelled**](classendstone_1_1ICancellable.md#function-iscancelled) () const = 0<br> |
| virtual [**void**](classendstone_1_1Vector.md) | [**setCancelled**](classendstone_1_1ICancellable.md#function-setcancelled) ([**bool**](classendstone_1_1Vector.md) cancel) = 0<br> |
| virtual  | [**~ICancellable**](classendstone_1_1ICancellable.md#function-icancellable) () = default<br> |
















































































## Public Static Attributes Documentation




### variable NAME 

```C++
const std::string endstone::PacketReceiveEvent::NAME;
```




<hr>
## Public Functions Documentation




### function PacketReceiveEvent 

```C++
inline endstone::PacketReceiveEvent::PacketReceiveEvent (
    Player & player,
    int packet_id,
    std::string_view payload
) 
```




<hr>



### function getEventName 

```C++
inline virtual std::string endstone::PacketReceiveEvent::getEventName () override const
```



Gets a user-friendly identifier for this event.




**Returns:**

name of this event 





        
Implements [*endstone::Event::getEventName*](classendstone_1_1Event.md#function-geteventname)


<hr>



### function getPacketId 

_Gets the ID of the packet._ 
```C++
inline int endstone::PacketReceiveEvent::getPacketId () const
```





**Returns:**

The packet ID. 





        

<hr>



### function getPayload 

_Gets the raw packet data_ **excluding** _the header._
```C++
inline std::string_view endstone::PacketReceiveEvent::getPayload () const
```





**Returns:**

The packet payload data. 





        

<hr>



### function getPlayer 

```C++
inline Player & endstone::PacketReceiveEvent::getPlayer () const
```



Returns the player involved in this event




**Returns:**

[**Player**](classendstone_1_1Player.md) who is involved in this event 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `include/endstone/event/server/packet_receive_event.h`

