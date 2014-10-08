#ifndef ASTERISKMANAGER_H
#define ASTERISKMANAGER_H

#include <QTcpSocket>
#include <QMetaEnum>

class AsteriskManager : public QTcpSocket
{
    Q_OBJECT

    Q_ENUMS(Response)
    Q_ENUMS(Event)
    Q_ENUMS(Action)

public:
    enum Response {
        Success,
        Error,
        Follows
    };

    enum Event {
        AgentCalled,
        AgentComplete,
        AgentConnect,
        AgentDump,
        Agentlogin,
        Agentlogoff,
        AgentRingNoAnswer,
        Alarm,
        AlarmClear,
        Bridge,
        BridgeAction,
        BridgeExec,
        ChanSpyStart,
        ChanSpyStop,
        ConfbridgeEnd,
        ConfbridgeJoin,
        ConfbridgeLeave,
        ConfbridgeStart,
        ConfbridgeTalking,
        DAHDIChannel,
        Dial,
        DNDState,
        DTMF,
        ExtensionStatus,
        FullyBooted,
        Hangup,
        HangupHandlerPop,
        HangupHandlerPush,
        HangupHandlerRun,
        HangupRequest,
        Hold,
        Join,
        Leave,
        LocalBridge,
        LogChannel,
        Masquerade,
        MeetmeEnd,
        MeetmeJoin,
        MeetmeLeave,
        MeetmeMute,
        MeetmeTalking,
        MeetmeTalkRequest,
        MessageWaiting,
        ModuleLoadReport,
        NewAccountCode,
        NewCallerid,
        Newchannel,
        Newexten,
        NewPeerAccount,
        Newstate,
        OriginateResponse,
        ParkedCall,
        ParkedCallGiveUp,
        ParkedCallTimeOut,
        Pickup,
        PresenceStatus,
        QueueCallerAbandon,
        QueueMemberAdded,
        QueueMemberPaused,
        QueueMemberPenalty,
        QueueMemberRemoved,
        QueueMemberRinginuse,
        QueueMemberStatus,
        Rename,
        Shutdown,
        SoftHangupRequest,
        SpanAlarm,
        SpanAlarmClear,
        UnParkedCall,
        UserEvent,
        VarSet
    };

    enum Action {
        AbsoluteTimeout,
        AgentLogoff,
        Agents,
        AGI,
        AOCMessage,
        Atxfer,
//        Bridge,
        Challenge,
        ChangeMonitor,
        Command,
        ConfbridgeKick,
        ConfbridgeList,
        ConfbridgeListRooms,
        ConfbridgeLock,
        ConfbridgeMute,
        ConfbridgeSetSingleVideoSrc,
        ConfbridgeStartRecord,
        ConfbridgeStopRecord,
        ConfbridgeUnlock,
        ConfbridgeUnmute,
        CoreSettings,
        CoreShowChannels,
        CoreStatus,
        CreateConfig,
        DAHDIDialOffhook,
        DAHDIDNDoff,
        DAHDIDNDon,
        DAHDIHangup,
        DAHDIRestart,
        DAHDIShowChannels,
        DAHDITransfer,
        DataGet,
        DBDel,
        DBDelTree,
        DBGet,
        DBPut,
        Events,
        ExtensionState,
        Filter,
        FilterList,
        GetConfig,
        GetConfigJSON,
        Getvar,
//        Hangup,
        IAXnetstats,
        IAXpeerlist,
        IAXpeers,
        IAXregistry,
        JabberSend,
        JabberSend_res_jabber,
        JabberSend_res_xmpp,
        ListCategories,
        ListCommands,
        LocalOptimizeAway,
        Login,
        Logoff,
        MailboxCount,
        MailboxStatus,
        MeetmeList,
        MeetmeListRooms,
//        MeetmeMute,
        MeetmeUnmute,
        MessageSend,
        MixMonitor,
        MixMonitorMute,
        ModuleCheck,
        ModuleLoad,
        Monitor,
        MuteAudio,
        Originate,
        Park,
        ParkedCalls,
        Parkinglots,
        PauseMonitor,
        Ping,
        PlayDTMF,
        PresenceState,
        PRIShowSpans,
        QueueAdd,
        QueueLog,
        QueueMemberRingInUse,
        QueuePause,
        QueuePenalty,
        QueueReload,
        QueueRemove,
        QueueReset,
        QueueRule,
        Queues,
        QueueStatus,
        QueueSummary,
        Redirect,
        Reload,
        SendText,
        Setvar,
        ShowDialPlan,
        SIPnotify,
        SIPpeers,
        SIPpeerstatus,
        SIPqualifypeer,
        SIPshowpeer,
        SIPshowregistry,
        SKINNYdevices,
        SKINNYlines,
        SKINNYshowdevice,
        SKINNYshowline,
        Status,
        StopMixMonitor,
        StopMonitor,
        UnpauseMonitor,
        UpdateConfig,
//        UserEvent,
        VoicemailUsersList,
        WaitEvent
    };

    explicit AsteriskManager(QObject *parent = 0);

    void actionAbsoluteTimeout();
    void actionAgentLogoff();
    void actionAgents();
    void actionAGI();
    void actionAOCMessage();
    void actionAtxfer();
    void actionBridge();
    void actionChallenge();
    void actionChangeMonitor();
    void actionCommand();
    void actionConfbridgeKick();
    void actionConfbridgeList();
    void actionConfbridgeListRooms();
    void actionConfbridgeLock();
    void actionConfbridgeMute();
    void actionConfbridgeSetSingleVideoSrc();
    void actionConfbridgeStartRecord();
    void actionConfbridgeStopRecord();
    void actionConfbridgeUnlock();
    void actionConfbridgeUnmute();
    void actionCoreSettings();
    void actionCoreShowChannels();
    void actionCoreStatus();
    void actionCreateConfig();
    void actionDAHDIDialOffhook();
    void actionDAHDIDNDoff();
    void actionDAHDIDNDon();
    void actionDAHDIHangup();
    void actionDAHDIRestart();
    void actionDAHDIShowChannels();
    void actionDAHDITransfer();
    void actionDataGet();
    void actionDBDel();
    void actionDBDelTree();
    void actionDBGet();
    void actionDBPut();
    void actionEvents();
    void actionExtensionState();
    void actionFilter();
    void actionFilterList();
    void actionGetConfig();
    void actionGetConfigJSON();
    void actionGetvar();
    void actionHangup(QString channel, uint cause = 0, QString actionID = QString());
    void actionIAXnetstats();
    void actionIAXpeerlist();
    void actionIAXpeers();
    void actionIAXregistry();
    void actionJabberSend();
    void actionJabberSend_res_jabber();
    void actionJabberSend_res_xmpp();
    void actionListCategories();
    void actionListCommands();
    void actionLocalOptimizeAway();
    void actionLogin(QString username, QString secret = QString(), QString actionID = QString());
    void actionLogoff(QString actionID = QString());
    void actionMailboxCount();
    void actionMailboxStatus();
    void actionMeetmeList();
    void actionMeetmeListRooms();
    void actionMeetmeMute();
    void actionMeetmeUnmute();
    void actionMessageSend();
    void actionMixMonitor();
    void actionMixMonitorMute();
    void actionModuleCheck();
    void actionModuleLoad();
    void actionMonitor();
    void actionMuteAudio();
    void actionOriginate(QString channel,
                         QString exten = QString(),
                         QString context = QString(),
                         uint priority = 1,
                         QString application = QString(),
                         QString data = QString(),
                         uint timeout = 0,
                         QString callerID = QString(),
                         QVariantMap variables = QVariantMap(),
                         QString account = QString(),
                         bool earlyMedia = false,
                         bool async = false,
                         QStringList codecs = QStringList(),
                         QString actionID = QString());
    void actionPark();
    void actionParkedCalls();
    void actionParkinglots();
    void actionPauseMonitor();
    void actionPing();
    void actionPlayDTMF();
    void actionPresenceState();
    void actionPRIShowSpans();
    void actionQueueAdd();
    void actionQueueLog();
    void actionQueueMemberRingInUse();
    void actionQueuePause();
    void actionQueuePenalty();
    void actionQueueReload();
    void actionQueueRemove();
    void actionQueueReset();
    void actionQueueRule();
    void actionQueues();
    void actionQueueStatus();
    void actionQueueSummary();
    void actionRedirect();
    void actionReload();
    void actionSendText();
    void actionSetvar();
    void actionShowDialPlan();
    void actionSIPnotify();
    void actionSIPpeers();
    void actionSIPpeerstatus();
    void actionSIPqualifypeer();
    void actionSIPshowpeer();
    void actionSIPshowregistry();
    void actionSKINNYdevices();
    void actionSKINNYlines();
    void actionSKINNYshowdevice();
    void actionSKINNYshowline();
    void actionStatus();
    void actionStopMixMonitor();
    void actionStopMonitor();
    void actionUnpauseMonitor();
    void actionUpdateConfig();
    void actionUserEvent();
    void actionVoicemailUsersList();
    void actionWaitEvent ();

private:
    QMetaEnum responseEnum, eventEnum, actionEnum;
    QVariantMap packetBuffer;
    QHash<QString, Action> actions;

    QString valueToString(QVariant value);
    QVariant stringValue(QString string);
    Action fetchAction(QString actionID);
    void insertNotEmpty(QVariantMap *headers, QString key, QVariant value);
    void dispatchPacket();
    void sendAction(Action action, QVariantMap headers);

private slots:
    void onReadyRead();

signals:
    void connected(QString version);
    void responseSent(AsteriskManager::Action action, AsteriskManager::Response response, QVariantMap headers);
    void eventGenerated(AsteriskManager::Event event, QVariantMap headers);
};

#endif // ASTERISKMANAGER_H
