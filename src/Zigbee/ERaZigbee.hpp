#ifndef INC_ERA_ZIGBEE_HPP_
#define INC_ERA_ZIGBEE_HPP_

#include <vector>
#include <cmath>
#include <ERa/ERaParam.hpp>
#include <ERa/ERaTimer.hpp>
#include <Utility/ERaUtility.hpp>
#include <Zigbee/ERaZigbeeState.hpp>
#include <Zigbee/ERaToZigbee.hpp>
#include <Zigbee/ERaFromZigbee.hpp>
#include "utility/ERaUtilityZigbee.hpp"
#include "ERaZigbeeConfig.hpp"

InfoDevice_t* InfoDevice_t::instance = nullptr;
InfoCoordinator_t* InfoCoordinator_t::instance = nullptr;

template <class Api>
class ERaZigbee
    : public ERaToZigbee< ERaZigbee<Api> >
    , public ERaFromZigbee< ERaZigbee<Api> >
{
    const char* TAG = "Zigbee";
	const uint8_t SOF = 0xFE;
	const uint8_t DataStart = 4;

    const uint8_t PositionSOF = 0;
	const uint8_t PositionDataLength = 1;
	const uint8_t PositionCmd0 = 2;
	const uint8_t PositionCmd1 = 3;
    
	const uint8_t MinMessageLength = 5;
	const uint8_t MaxDataSize = 250;

    const uint8_t BEACON_MAX_DEPTH = 0x0F;
	const OptionsAFT Options = OptionsAFT::DEFAULT_OP;
	const uint8_t Radius = 2 * BEACON_MAX_DEPTH; // 0x1E

    const ZBChannelT DefaultChannel = ZBChannelT::CHANNEL_11;
    const ZBChannelT ChannelList[16] = {ZBChannelT::CHANNEL_11, ZBChannelT::CHANNEL_12, ZBChannelT::CHANNEL_13, ZBChannelT::CHANNEL_14, ZBChannelT::CHANNEL_15,
                                        ZBChannelT::CHANNEL_16, ZBChannelT::CHANNEL_17, ZBChannelT::CHANNEL_18, ZBChannelT::CHANNEL_19, ZBChannelT::CHANNEL_20,
                                        ZBChannelT::CHANNEL_21, ZBChannelT::CHANNEL_22, ZBChannelT::CHANNEL_23, ZBChannelT::CHANNEL_24, ZBChannelT::CHANNEL_25,
                                        ZBChannelT::CHANNEL_26};
    const uint8_t TcLinkKey[32] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                            0x5a, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6c, 0x6c, 0x69, 0x61, 0x6e, 0x63, 0x65, 0x30, 0x39,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /* ZigBeeAlliance09 */

    typedef void* QueueMessage_t;
    friend class ERaToZigbee< ERaZigbee<Api> >;
    friend class ERaFromZigbee< ERaZigbee<Api> >;
    typedef ERaToZigbee< ERaZigbee<Api> > ToZigbee;
    typedef ERaFromZigbee< ERaZigbee<Api> > FromZigbee;

public:
    ERaZigbee()
        : stream(NULL)
        , messageHandle(NULL)
        , timerPing()
        , timerJoin()
        , device(InfoDevice_t::instance)
        , coordinator(InfoCoordinator_t::instance)
    {}
    ~ERaZigbee()
    {}

protected:
    void begin() { 
        this->configZigbee();
        this->initZigbee(false);
        this->timerPing = this->timer.setInterval(PING_INTERVAL, [=](void* args) {
            this->zigbeeTimerCallback(args);
        }, &this->timerPing);
    }

    void run() {
        for (;;) {
            switch (ZigbeeState::get()) {
                case ZigbeeStateT::STATE_ZB_INIT_SUCCESSFUL:
                    ZigbeeState::set(ZigbeeStateT::STATE_ZB_RUNNING);
                    break;
                case ZigbeeStateT::STATE_ZB_DEVICE_JOINED:
                    if (!this->timerJoined) {
                        this->timerJoined = this->timer.setTimeout(JOIN_TIMEOUT, [=](void* args) {
                            this->zigbeeTimerCallback(args);
                        }, &this->timerJoined);
                    }
                case ZigbeeStateT::STATE_ZB_RUNNING:
                case ZigbeeStateT::STATE_ZB_PERMIT_JOIN:
                    this->handleZigbeeData();
                    break;
                case ZigbeeStateT::STATE_ZB_DEVICE_INTERVIEWING:
                    this->interviewDevice();
                    break;
                case ZigbeeStateT::STATE_ZB_FACTORY_RESET:
                    this->factoryResetZigbee();
                    break;
                case ZigbeeStateT::STATE_ZB_INIT_FAIL:
                case ZigbeeStateT::STATE_ZB_INIT_FORMAT:
                case ZigbeeStateT::STATE_ZB_INIT_MAX:
                default:
                    this->initZigbee(false);
                    break;
            }
            this->timer.run();
            ERaDelay(10);
        }
    }

    void runControl() {
        for (;;) {
            switch (ZigbeeState::get()) {
                case ZigbeeStateT::STATE_ZB_DEVICE_INTERVIEWING:
                case ZigbeeStateT::STATE_ZB_FACTORY_RESET:
                case ZigbeeStateT::STATE_ZB_INIT_FAIL:
                case ZigbeeStateT::STATE_ZB_INIT_FORMAT:
                case ZigbeeStateT::STATE_ZB_INIT_MAX:
                    break;
                default:
                    this->getZigbeeAction();
                    break;
            }
            ERaDelay(10);
        }
    }

    void runResponse() {
        for (;;) {
            switch (ZigbeeState::get()) {
                case ZigbeeStateT::STATE_ZB_FACTORY_RESET:
                case ZigbeeStateT::STATE_ZB_INIT_FAIL:
                case ZigbeeStateT::STATE_ZB_INIT_FORMAT:
                case ZigbeeStateT::STATE_ZB_INIT_MAX:
                    break;
                default:
                    this->handleDefaultResponse();
                    break;
            }
            ERaDelay(10);
        }
    }

private:
    void configZigbee();
    void initZigbee(bool format, bool invalid = false);
    void startZigbee(bool& format, bool& invalid);
    void factoryResetZigbee();
    void handleZigbeeData();
    void handleDefaultResponse();
    bool processZigbee(uint8_t* buffer,
                        int length,
                        int maxLength,
                        uint8_t* payload,
                        uint8_t index,
                        uint8_t zStackLength,
                        uint8_t* cmdStatus = nullptr,
                        Response_t* rspWait = nullptr,
                        void* value = nullptr);
    bool interviewDevice();
    void removeDevice(const cJSON* const root, AFAddrType_t& dstAddr);
    void removeDeviceWithAddr(AFAddrType_t& dstAddr);
    void pingCoordinator();
    void createInfoCoordinator();
    ResultT permitJoinDuration(AFAddrType_t& dstAddr, uint8_t seconds);
    ResultT readAttrDevice(AFAddrType_t& dstAddr,
                            ClusterIDT zclId,
                            vector<uint16_t> attributes,
                            size_t limit = 2,
                            void* value = nullptr);
    ResultT writeAttrDevice(AFAddrType_t& dstAddr,
                            ClusterIDT zclId,
                            vector<DataWriteAttr_t> data,
                            size_t limit = 2);

    template <int inSize, int outSize>
    bool isClusterExist(const ClusterIDT(&inZcl)[inSize], const ClusterIDT(&outZcl)[outSize], const ClusterIDT zclId);
    template <int size>
    bool isBindReportExist(const ConfigBindReport_t(&inConfig)[size], const ClusterIDT zclId);
    template <typename T, int size>
    bool isElementExist(const T(&elementList)[size], const T element);

    void publishZigbeeData(const IdentDeviceAddr_t* deviceInfo);
    void publishZigbeeData(const char* topic, cJSON* payload);
    bool actionZigbee(const ZigbeeActionT type, const char* ieeeAddr, const cJSON* const payload);
    void getZigbeeAction();
    bool addZigbeeAction(const ZigbeeActionT type, const char* ieeeAddr, const cJSON* const payload);
    void zigbeeTimerCallback(void* args);

	bool isRequest() {
		return this->queue.readable();
	}

	ZigbeeAction_t& getRequest() {
		return this->queue;
	}

	ERaQueue<ZigbeeAction_t, 10> queue;
    Stream* stream;
    QueueMessage_t messageHandle;

    ERaTimer timer;
    ERaTimer::iterator timerPing;
    ERaTimer::iterator timerJoin;
    ERaTimer::iterator timerJoined;

    InfoDevice_t*& device;
    InfoCoordinator_t*& coordinator;
};

template <class Api>
void ERaZigbee<Api>::initZigbee(bool format, bool invalid) {
    if (ZigbeeState::get() == ZigbeeStateT::STATE_ZB_INIT_FORMAT) {
        format = true;
    }
    do {
        switch (ZigbeeState::get()) {
            case ZigbeeStateT::STATE_ZB_INIT_SUCCESSFUL:
                break;
            default:
                this->startZigbee(format, invalid);
                break;
        }
        ERaDelay(1000);
    } while (ZigbeeState::get() != ZigbeeStateT::STATE_ZB_INIT_SUCCESSFUL);
}

template <class Api>
void ERaZigbee<Api>::handleDefaultResponse() {
    static DefaultRsp_t rsp {};
    static SyncRsp_t syncRsp {};

    syncRsp = {
        .transId = rsp.transId,
        .frameCtrl = rsp.frameCtrl,
        .dstAddr = rsp.dstAddr,
        .dstEndpoint = rsp.dstEndpoint,
        .srcEndPoint = rsp.srcEndpoint,
        .zclId = rsp.zclId
    };

    switch (rsp.zclId) {
        default:
            if (ZigbeeState::get() == ZigbeeStateT::STATE_ZB_DEVICE_INTERVIEWING) {
                if (rsp.dstAddr != this->device->address.addr.nwkAddr) {
                    break;
                }
            }
            if (rsp.isFirst) {
                ToZigbee::CommandZigbee::defaultResponse(rsp, ZnpCommandStatusT::SUCCESS_STATUS);
            }
            break;
    }
}

template <class Api>
bool ERaZigbee<Api>::processZigbee(uint8_t* buffer,
                                    int length,
                                    int maxLength,
                                    uint8_t* payload,
                                    uint8_t index,
                                    uint8_t zStackLength,
                                    uint8_t* cmdStatus,
                                    Response_t* rspWait,
                                    void* value) {
    if (!length) {
        return false;
    }
    Response_t rsp {0};
    for (int i = 0; i < length; ++i) {
        uint8_t b = buffer[i];

        if (index >= maxLength) {
            index = 0;
            continue;
        }
        if (b == this->SOF && !index) {
            payload[index++] = b;
            continue;
        }
        if (index == 1) {
            zStackLength = b;
        }
        if (index) {
            payload[index++] = b;
        }
        if (index && (payload[this->PositionSOF] != this->SOF || zStackLength > this->MaxDataSize)) {
            index = 0;
            continue;
        }
        if (index == zStackLength + this->MinMessageLength) {
            if (payload[this->PositionSOF] == this->SOF) {
                rsp = FromZigbee::fromZigbee(payload, value);
                if (rsp.type != TypeT::ERR) {
                    if (rspWait == nullptr) {
                        // sync
                    }
                    else {
                        if (cmdStatus != nullptr) {
                            if (CheckAFdata_t(rsp, *rspWait)) {
                                *cmdStatus = rsp.cmdStatus;
                            }
                            if (CompareRsp_t(rsp, *rspWait)) {
                                return true;
                            }
                            if (CheckRsp_t(rsp, *rspWait)) {
                                // sync
                            }
                        }
                    }
                }
            }
            index = 0;
            zStackLength = 0;
            memset(payload, 0, length);
        }
    }
    return false;
}

template <class Api>
void ERaZigbee<Api>::publishZigbeeData(const IdentDeviceAddr_t* deviceInfo) {
    if (deviceInfo == nullptr ||
        deviceInfo == std::end(this->coordinator->deviceIdent)) {
        return;
    }
    static_cast<Api*>(this)->zigbeeDataWrite(deviceInfo->data.topic, deviceInfo->data.payload);
}

template <class Api>
void ERaZigbee<Api>::publishZigbeeData(const char* topic, cJSON* payload) {
    if (topic == nullptr || payload == nullptr) {
        return;
    }
    static_cast<Api*>(this)->zigbeeDataWrite(topic, payload);
}

template <class Api>
bool ERaZigbee<Api>::actionZigbee(const ZigbeeActionT type, const char* ieeeAddr, const cJSON* const payload) {
    if (ieeeAddr == nullptr || payload == nullptr) {
        return false;
    }

    AFAddrType_t dstAddr {0};

    switch (type) {
        case ZigbeeActionT::ZIGBEE_ACTION_SET:
        case ZigbeeActionT::ZIGBEE_ACTION_GET:
            if (CompareString(ieeeAddr, "group")) {
                dstAddr.addrMode = AddressModeT::ADDR_GROUP;
            }
            else {
                dstAddr.addrMode = AddressModeT::ADDR_16BIT;
                StringToIEEE(ieeeAddr, dstAddr.addr.ieeeAddr);
            }
            ToZigbee::toZigbee(payload, dstAddr, ((type == ZigbeeActionT::ZIGBEE_ACTION_SET) 
                                                ? ToZigbee::ConvertToZigbeeT::CONVERT_SET_TO_ZIGBEE
                                                : ToZigbee::ConvertToZigbeeT::CONVERT_GET_TO_ZIGBEE));
            break;
        case ZigbeeActionT::ZIGBEE_ACTION_PERMIT_JOIN:
            ToZigbee::permitJoinToZigbee(payload);
            break;
        case ZigbeeActionT::ZIGBEE_ACTION_REMOVE_DEVICE:
            this->removeDevice(payload, dstAddr);
            break;
        default:
            break;
    }

    return true;
}

template <class Api>
void ERaZigbee<Api>::getZigbeeAction() {
    if (!this->isRequest()) {
        return;
    }
    ZigbeeAction_t& req = this->getRequest();
    if (req.ieeeAddr == nullptr) {
        return;
    }
    if (req.payload == nullptr) {
        free(req.ieeeAddr);
        req.ieeeAddr = nullptr;
        return;
    }
    this->actionZigbee(req.type, req.ieeeAddr, req.payload);
    free(req.ieeeAddr);
    cJSON_Delete(req.payload);
    req.ieeeAddr = nullptr;
    req.payload = nullptr;
}

template <class Api>
bool ERaZigbee<Api>::addZigbeeAction(const ZigbeeActionT type, const char* ieeeAddr, const cJSON* const payload) {
    if (ieeeAddr == nullptr || payload == nullptr) {
        return false;
    }
	if (!this->queue.writeable()) {
        return false;
    }
    ZigbeeAction_t req;
    req.type = type;
    req.ieeeAddr = (char*)ERA_MALLOC(strlen(ieeeAddr) + 1);
    req.payload = payload;
    memset(req.ieeeAddr, 0, strlen(ieeeAddr) + 1);
    strcpy(req.ieeeAddr, ieeeAddr);
    this->queue += req;
    return true;
}

#include "adapter/ERaReportingZigbee.hpp"
#include "adapter/ERaEndpointsZigbee.hpp"
#include "adapter/ERaAdapterZigbee.hpp"
#include "adapter/ERaDeviceZigbee.hpp"

#endif /* INC_ERA_ZIGBEE_HPP_ */