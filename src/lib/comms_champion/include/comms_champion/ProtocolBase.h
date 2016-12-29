//
// Copyright 2015 - 2016 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include <algorithm>
#include <iterator>
#include <cassert>


#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QByteArray>
CC_ENABLE_WARNINGS()

#include "comms_champion/ErrorStatus.h"
#include "comms/util/ScopeGuard.h"
#include "comms/util/Tuple.h"

#include "Protocol.h"
#include "Message.h"
#include "RawDataMessage.h"
#include "InvalidMessage.h"
#include "ExtraInfoMessage.h"

namespace comms_champion
{

template <
    typename TProtStack,
    typename TTransportMsg,
    typename TRawDataMsg = RawDataMessage<TProtStack> >
class ProtocolBase : public Protocol
{
protected:
    ProtocolBase() = default;

    typedef TProtStack ProtocolStack;
    typedef TTransportMsg TransportMsg;
    typedef TRawDataMsg RawDataMsg;
    typedef typename ProtocolStack::Message ProtocolMessage;
    typedef typename ProtocolMessage::MsgIdType MsgIdType;
    typedef typename ProtocolMessage::MsgIdParamType MsgIdParamType;
    typedef typename ProtocolStack::AllMessages AllMessages;
    typedef InvalidMessage<ProtocolMessage> InvalidMsg;
    typedef ExtraInfoMessage<ProtocolMessage> ExtraInfoMsg;

    static_assert(
        !std::is_void<AllMessages>::value,
        "AllMessages must be a normal type");

    static_assert(
        comms::util::IsTuple<AllMessages>::Value,
        "AllMessages is expected to be a tuple.");


    virtual MessagesList readImpl(const DataInfo& dataInfo, bool final) override
    {
        const std::uint8_t* iter = &dataInfo.m_data[0];
        auto size = dataInfo.m_data.size();

        MessagesList allMsgs;
        m_data.reserve(m_data.size() + size);
        std::copy_n(iter, size, std::back_inserter(m_data));

        using ReadIterator = typename ProtocolStack::ReadIterator;
        ReadIterator readIterBeg = &m_data[0];

        auto remainingSizeCalc =
            [this](ReadIterator readIter) -> std::size_t
            {
                ReadIterator const dataBegin = &m_data[0];
                auto consumed =
                    static_cast<std::size_t>(
                        std::distance(dataBegin, readIter));
                assert(consumed <= m_data.size());
                return m_data.size() - consumed;
            };

        auto eraseGuard =
            comms::util::makeScopeGuard(
                [this, &readIterBeg]()
                {
                    ReadIterator dataBegin = &m_data[0];
                    auto dist =
                        static_cast<std::size_t>(
                            std::distance(dataBegin, readIterBeg));
                    m_data.erase(m_data.begin(), m_data.begin() + dist);
                });

        auto setExtraInfoFunc =
            [&dataInfo](Message& msg)
            {
                if (dataInfo.m_extraProperties.isEmpty()) {
                    return;
                }

                auto jsonObj = QJsonObject::fromVariantMap(dataInfo.m_extraProperties);
                QJsonDocument doc(jsonObj);

                std::unique_ptr<ExtraInfoMsg> extraInfoMsgPtr(new ExtraInfoMsg());
                auto& str = std::get<0>(extraInfoMsgPtr->fields());
                str.value() = doc.toJson().constData();
                setExtraInfoToMessageProperties(dataInfo.m_extraProperties, msg);
                setExtraInfoMsgToMessageProperties(
                    MessagePtr(extraInfoMsgPtr.release()),
                    msg);
            };

        auto checkGarbageFunc =
            [this, &allMsgs, &setExtraInfoFunc]()
            {
                if (!m_garbage.empty()) {
                    MessagePtr invalidMsgPtr(new InvalidMsg());
                    setNameToMessageProperties(*invalidMsgPtr);
                    std::unique_ptr<RawDataMsg> rawDataMsgPtr(new RawDataMsg());
                    ReadIterator garbageReadIterator = &m_garbage[0];
                    auto esTmp = rawDataMsgPtr->read(garbageReadIterator, m_garbage.size());
                    static_cast<void>(esTmp);
                    assert(esTmp == comms::ErrorStatus::Success);
                    setRawDataToMessageProperties(MessagePtr(rawDataMsgPtr.release()), *invalidMsgPtr);
                    setExtraInfoFunc(*invalidMsgPtr);
                    allMsgs.push_back(std::move(invalidMsgPtr));
                    m_garbage.clear();
                }
            };

        while (true) {
            using ProtocolMsgPtr = typename ProtocolStack::MsgPtr;

            ProtocolMsgPtr msgPtr;

            auto readIterCur = readIterBeg;
            auto remainingSize = remainingSizeCalc(readIterCur);
            if (remainingSize == 0U) {
                break;
            }

            auto es =
                m_protStack.read(
                    msgPtr,
                    readIterCur,
                    remainingSize);

            if (es == comms::ErrorStatus::NotEnoughData) {
                break;
            }

            auto addMsgInfoGuard =
                comms::util::makeScopeGuard(
                    [this, &allMsgs, &msgPtr]()
                    {
                        assert(msgPtr);
                        setNameToMessageProperties(*msgPtr);
                        allMsgs.push_back(MessagePtr(std::move(msgPtr)));
                    });

            auto setExtrasFunc =
                [readIterBeg, &readIterCur, &msgPtr, &setExtraInfoFunc]()
                {
                    // readIterBeg is captured by value on purpose
                    auto dataSize = static_cast<std::size_t>(
                                std::distance(readIterBeg, readIterCur));

                    auto readTransportIterBegTmp = readIterBeg;
                    std::unique_ptr<TransportMsg> transportMsgPtr(new TransportMsg());
                    auto esTmp = transportMsgPtr->read(readTransportIterBegTmp, dataSize);
                    static_cast<void>(esTmp);
                    assert(esTmp == comms::ErrorStatus::Success);
                    setTransportToMessageProperties(MessagePtr(transportMsgPtr.release()), *msgPtr);

                    auto readRawIterBegTmp = readIterBeg;
                    std::unique_ptr<RawDataMsg> rawDataMsgPtr(new RawDataMsg());
                    esTmp = rawDataMsgPtr->read(readRawIterBegTmp, dataSize);
                    static_cast<void>(esTmp);
                    assert(esTmp == comms::ErrorStatus::Success);
                    setRawDataToMessageProperties(MessagePtr(rawDataMsgPtr.release()), *msgPtr);
                    setExtraInfoFunc(*msgPtr);
                };

            if (es == comms::ErrorStatus::Success) {
                checkGarbageFunc();
                assert(msgPtr);
                setExtrasFunc();
                readIterBeg = readIterCur;
                continue;
            }

            if (es == comms::ErrorStatus::InvalidMsgData) {
                checkGarbageFunc();
                msgPtr.reset(new InvalidMsg());
                setExtrasFunc();
                readIterBeg = readIterCur;
                continue;
            }

            addMsgInfoGuard.release();

            if (es == comms::ErrorStatus::MsgAllocFailure) {
                assert(!"Mustn't happen");
                break;
            }

            // Protocol error
            m_garbage.push_back(*readIterBeg);
            static const std::size_t GarbageLimit = 512;
            if (GarbageLimit <= m_garbage.size()) {
                checkGarbageFunc();
            }
            ++readIterBeg;
        }

        if (final) {
            ReadIterator dataBegin = &m_data[0];
            auto consumed =
                static_cast<std::size_t>(std::distance(dataBegin, readIterBeg));
            auto remDataCount = m_data.size() - consumed;
            m_garbage.insert(m_garbage.end(), m_data.begin() + consumed, m_data.end());
            std::advance(readIterBeg, remDataCount);
            checkGarbageFunc();
        }
        return allMsgs;
    }

    virtual DataInfoPtr writeImpl(Message& msg) override
    {
        DataInfo::DataSeq data;
        auto writeIter = std::back_inserter(data);
        auto es =
            m_protStack.write(
                static_cast<const ProtocolMessage&>(msg),
                writeIter,
                data.max_size());
        if (es == comms::ErrorStatus::UpdateRequired) {
            auto updateIter = &data[0];
            es = m_protStack.update(updateIter, data.size());
        }

        if (es != comms::ErrorStatus::Success) {
            assert(!"Unexpected write/update failure");
            return DataInfoPtr();
        }

        auto dataInfo = makeDataInfo();
        assert(dataInfo);

        dataInfo->m_timestamp = DataInfo::TimestampClock::now();
        dataInfo->m_data = std::move(data);
        return dataInfo;
    }

    virtual UpdateStatus updateMessageImpl(Message& msg) override
    {
        bool refreshed = msg.refreshMsg();

        assert(!msg.idAsString().isEmpty());
        do {
            std::vector<std::uint8_t> data;

            auto writeIter = std::back_inserter(data);
            auto es =
                m_protStack.write(
                    static_cast<const ProtocolMessage&>(msg),
                    writeIter,
                    data.max_size());
            if (es == comms::ErrorStatus::UpdateRequired) {
                auto updateIter = &data[0];
                es = m_protStack.update(updateIter, data.size());
            }

            if (es != comms::ErrorStatus::Success) {
                assert(!"Message write/update has failed unexpectedly");
                break;
            }

            auto readMessageFunc =
                [&data](ProtocolMessage& msgToRead) -> bool
                {
                    typename ProtocolMessage::ReadIterator iter = nullptr;
                    if (!data.empty()) {
                        iter = &data[0];
                    }

                    auto esTmp = msgToRead.read(iter, data.size());
                    if (esTmp != comms::ErrorStatus::Success) {
                        return false;
                    }

                    return true;
                };

            std::unique_ptr<TransportMsg> transportMsgPtr(new TransportMsg());
            if (!readMessageFunc(*transportMsgPtr)) {
                assert(!"Unexpected failure to read transport message");
                break;
            }

            std::unique_ptr<RawDataMsg> rawDataMsgPtr(new RawDataMsg());
            if (!readMessageFunc(*rawDataMsgPtr)) {
                assert(!"Unexpected failure to read raw data of the message");
                break;
            }

            setTransportToMessageProperties(MessagePtr(transportMsgPtr.release()), msg);
            setRawDataToMessageProperties(MessagePtr(rawDataMsgPtr.release()), msg);

            auto extraProps = getExtraInfoFromMessageProperties(msg);
            if (extraProps.isEmpty()) {
                setExtraInfoMsgToMessageProperties(MessagePtr(), msg);
                break;
            }

            auto jsonObj = QJsonObject::fromVariantMap(extraProps);
            QJsonDocument doc(jsonObj);

            std::unique_ptr<ExtraInfoMsg> extraInfoMsgPtr(new ExtraInfoMsg());
            auto& str = std::get<0>(extraInfoMsgPtr->fields());
            str.value() = doc.toJson().constData();
            setExtraInfoMsgToMessageProperties(
                MessagePtr(extraInfoMsgPtr.release()),
                msg);

        } while (false);

        if (refreshed) {
            return UpdateStatus::Changed;
        }

        return UpdateStatus::NoChange;
    }

    virtual MessagePtr cloneMessageImpl(const Message& msg) override
    {
        MessagePtr clonedMsg;
        unsigned idx = 0;
        while (true) {
            auto msgId = static_cast<const ProtocolMessage&>(msg).getId();
            clonedMsg = m_protStack.createMsg(msgId, idx);
            if (!clonedMsg) {
                break;
            }

            if (clonedMsg->assign(msg)) {
                break;
            }

            clonedMsg.reset();
            ++idx;
        }
        return std::move(clonedMsg);
    }

    virtual MessagePtr createInvalidMessageImpl() override
    {
        MessagePtr msg(new InvalidMsg());
        setNameToMessageProperties(*msg);
        return msg;
    }

    virtual MessagePtr createRawDataMessageImpl() override
    {
        return MessagePtr(new RawDataMsg());
    }

    virtual MessagePtr createExtraInfoMessageImpl() override
    {
        return MessagePtr(new ExtraInfoMsg());
    }

    virtual MessagesList createAllMessagesImpl() override
    {
        return createAllMessagesInTuple<AllMessages>();
    }

    virtual MessagePtr createMessageImpl(const QString& idAsString, unsigned idx) override
    {
        return createMessageInternal(idAsString, idx, MsgIdTypeTag());
    }

    ProtocolStack& protocolStack()
    {
        return m_protStack;
    }

    const ProtocolStack& protocolStack() const
    {
        return m_protStack;
    }

    MessagePtr createMessage(MsgIdParamType id, unsigned idx = 0)
    {
        auto msgPtr = m_protStack.createMsg(id, idx);
        if (msgPtr) {
            setNameToMessageProperties(*msgPtr);
            updateMessage(*msgPtr);
        }
        return MessagePtr(std::move(msgPtr));
    }

    template <typename TMsgsTuple>
    MessagesList createAllMessagesInTuple()
    {
        MessagesList allMsgs;
        comms::util::tupleForEachType<TMsgsTuple>(AllMsgsCreateHelper(allMsgs));
        for (auto& msgPtr : allMsgs) {
            setNameToMessageProperties(*msgPtr);
            updateMessage(*msgPtr);
        }
        return allMsgs;
    }

private:
    struct NumericIdTag {};
    struct OtherIdTag {};

    typedef typename std::conditional<
        (std::is_enum<MsgIdType>::value || std::is_integral<MsgIdType>::value),
        NumericIdTag,
        OtherIdTag
    >::type MsgIdTypeTag;

    static_assert(std::is_same<MsgIdTypeTag, NumericIdTag>::value,
        "Non-numeric IDs are not supported properly yet.");

    class AllMsgsCreateHelper
    {
    public:
        AllMsgsCreateHelper(MessagesList& allMsgs)
          : m_allMsgs(allMsgs)
        {
        }

        template <typename TMsg>
        void operator()()
        {
            m_allMsgs.push_back(MessagePtr(new TMsg()));
        }

    private:
        MessagesList& m_allMsgs;
    };

    class MsgCreateHelper
    {
    public:
        MsgCreateHelper(const QString& id, unsigned idx, MessagePtr& msg)
          : m_id(id),
            m_reqIdx(idx),
            m_msg(msg)
        {
        }

        template <typename TMsg>
        void operator()()
        {
            if (m_msg) {
                return;
            }

            MessagePtr msgPtr(new TMsg());
            if (m_id != msgPtr->idAsString()) {
                return;
            }

            if (m_currIdx == m_reqIdx) {
                m_msg = std::move(msgPtr);
                return;
            }

            ++m_currIdx;
        }

    private:
        const QString& m_id;
        unsigned m_reqIdx;
        MessagePtr& m_msg;
        unsigned m_currIdx = 0;
    };

    MessagePtr createMessageInternal(const QString& idAsString, unsigned idx, NumericIdTag)
    {
        MessagePtr result;
        do {
            bool ok = false;
            int numId = idAsString.toInt(&ok, 10);
            if (!ok) {
                numId = idAsString.toInt(&ok, 16);
                if (!ok) {
                    break;
                }
            }

            result = createMessage(static_cast<MsgIdType>(numId), idx);
        } while (false);
        return result;
    }

    MessagePtr createMessageInternal(const QString& idAsString, unsigned idx, OtherIdTag)
    {
        MessagePtr result;
        comms::util::tupleForEachType<AllMessages>(MsgCreateHelper(name(), idAsString, idx, result));
        if (result) {
            updateMessage(*result);
        }
        return result;
    }

    ProtocolStack m_protStack;
    std::vector<std::uint8_t> m_data;
    std::vector<std::uint8_t> m_garbage;
};

}  // namespace comms_champion


