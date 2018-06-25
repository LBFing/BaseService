local FSMDouDiZhu = class("FSMDouDiZhu")

--[[
        �����������߼�״̬����
--]]

function FSMDouDiZhu:ctor()
    self._GameFSM 			= StateMachine:new();
    self._StateEnterTime    = 0;
    self._CurrState         = "TDDZStateWait";
    self.RoomDdz            = nil;
end

function FSMDouDiZhu:Init( room_ddz )
    self._GameFSM:setup_state({
        events = 
		{
            {name = "TDDZStateWait" 			        },  -- �ȴ���ʼ
			{name = "TDDZStateCheckStartGame"		    },  -- ����Ƿ���Կ�ʼ��Ϸ
			{name = "TDDZStateSelectMingCardStart" 	    },  -- ѡ�����ƿ�ʼ�׶�
			{name = "TDDZStateStartGame" 			    },  -- ��ʼ��Ϸ
            {name = "TDDZStateSendCard" 		    	},  -- ��������
            {name = "TDDZStateQiangDiZhu" 		    	},  -- �������׶�
            {name = "TDDZStateSelectAddTimes" 		    },  -- ѡ��ӱ��׶�
            {name = "TDDZStateAction" 		    	    },  -- ������ɻ
            {name = "TDDZStateOutCard" 		    	    },  -- ����״̬
            {name = "TDDZStateShowDown" 		    	},  -- ��Ϸ����
            {name = "TDDZStateRelieveRoom" 		    	},  -- ��ɢ����
        },
        callbacks =
		{
			onTDDZStateWait   		        = handler(self, self.DoWait),
			onTDDZStateCheckStartGame  	    = handler(self, self.DoCheckStartGame),
            onTDDZStateSelectMingCardStart  = handler(self, self.DoSelectMingCardStart),
            onTDDZStateStartGame  	        = handler(self, self.DoStartGame),
            onTDDZStateSendCard  	        = handler(self, self.DoSendCard),
            onTDDZStateQiangDiZhu  	        = handler(self, self.DoQiangDiZhu),
            onTDDZStateSelectAddTimes  	    = handler(self, self.DoSelectAddTimes),
            onTDDZStateAction  	            = handler(self, self.DoAction),
            onTDDZStateOutCard  	        = handler(self, self.DoOutCard),
            onTDDZStateShowDown  	        = handler(self, self.DoShowDown),
            onTDDZStateRelieveRoom  	    = handler(self, self.DoRelieveRoom),
		}
    })
    
    self.RoomDdz = room_ddz;
    self:SwitchState( self._CurrState );
end


function FSMDouDiZhu:__GetRunStateTime()
    return TimerMgr:GetTime() - self._StateEnterTime;
end

function FSMDouDiZhu:TickUpdate()
    self._GameFSM:do_event( self._CurrState, false );
end

function FSMDouDiZhu:SwitchState( event, ... )
    self._CurrState = event;
    self._StateEnterTime = TimerMgr:GetTime();
    self._GameFSM:do_event( event, true, ... );
end

function FSMDouDiZhu:GetState()
	return self._CurrState;
end

function FSMDouDiZhu:IsState( state )
    if self._CurrState == state then
        return true;
    end
	return false;
end

function FSMDouDiZhu:DoWait( event )
    if not event.args[1] then
        -- �����������ͨ��ʼ����ת�������������״̬��
        if self.RoomDdz:GameStartWait() then
            self:SwitchState("TDDZStateCheckStartGame");
        end
    end
end

-- ����Ƿ���Կ�ʼ,�ݲ���飬ֱ������һ״̬
function FSMDouDiZhu:DoCheckStartGame( event )
    self:SwitchState("TDDZStateStartGame");
end

-- �����ѡ�������ƿ�ʼ
function FSMDouDiZhu:DoSelectMingCardStart( event )
    if event.args[1] then
        self.RoomDdz:ResetGameData();
        self.RoomDdz:BroadcastGameInfo();
        self.RoomDdz:SendQiangDiZhuWik();
    end
end

function FSMDouDiZhu:DoStartGame( event )
    if event.args[1] then
        self.RoomDdz.IsGameStart    = true;
        self.RoomDdz:ResetGameData();
        self.RoomDdz:BroadcastGameInfo();
    else
        
        if self:__GetRunStateTime()<3000 then
            return;
        end
        
        -- self:SwitchState("TDDZStateSelectMingCardStart");
        self:SwitchState("TDDZStateSendCard");
    end
end

function FSMDouDiZhu:DoSendCard( event )
    if event.args[1] then
        self.RoomDdz:SendHandCard();
        self.RoomDdz:BroadcastGameInfo();
    else
        if self:__GetRunStateTime()<3000 then
            return;
        end
        
        self:SwitchState("TDDZStateQiangDiZhu");
    end
end

function FSMDouDiZhu:DoQiangDiZhu( event )
    if event.args[1] then
        self.RoomDdz:SendQiangDiZhuWik();
    end
end

function FSMDouDiZhu:DoSelectAddTimes( event )
    --if self:__GetRunStateTime()<10000 then
    --    return;
    --end
end

function FSMDouDiZhu:DoAction( event )
    if event.args[1] then
        --self.RoomDdz:SendQiangDiZhuWik();
    end
end

function FSMDouDiZhu:DoOutCard( event )
    if self:__GetRunStateTime()>3000 then
        self:SwitchState("TDDZStateAction");
    end
end

function FSMDouDiZhu:DoShowDown( event )
    nlinfo( "FSMClass:DoShowDown" );
end

function FSMDouDiZhu:DoRelieveRoom( event )
    nlinfo( "FSMClass:DoRelieveRoom" );
end

return FSMDouDiZhu;

