local RobotGameDdz = class("RobotGameDdz", RobotGameBase)

function RobotGameDdz:ctor()
    self.super:ctor();
    self.Robot = nil;
    self.RoomInfo   = nil;
    self.SelfData   = nil;          -- �������� player_list���Լ��ķ���������
end

function RobotGameDdz:GetFsmState()
    return self.Robot.GameFsm:GetState();
end

function RobotGameDdz:GetRobotState( enum_idx )
    return Misc.GetBit( self.SelfData.state, enum_idx );
end

function RobotGameDdz:DoCreatePrvRoom()
    local create_prvroom = { room_type="RM_DDZ", consume_id=1001, special_kind=0x1c5 };
    self.Robot:Send( "CPRM", "PB.MsgCreatePrivateRoom", create_prvroom );
end

function RobotGameDdz:DoJoinPrvRoom(open_room)
    local join_prvroom = { room_id=open_room.RoomID, room_type=open_room.RoomType };
    self.Robot:Send( "EPRM", "PB.MsgEnterPrivateRoom", join_prvroom );
end

function RobotGameDdz:DoReady()
    self.Robot:Send( "DDZ_SR" );
end

function RobotGameDdz:cbDdzGameInfo( msgin )
    
    local ddz_gi = msgin:rpb("PB.MsgDDZRoom");
    
    if ddz_gi==nil then
        nlwarning("ddz_gi==nil !!!!!!!!!!!!");
        return
    end

    -- ˢ�·���������
    self.RoomInfo = ddz_gi;
    
    -- ˢ���Լ�������
    for _,v in ipairs(ddz_gi.player_list) do
        if v.player_base.UID == self.Robot.Data.UID then
            self.SelfData = v;
        end
    end
    
    
    self.Robot:PrintTable(ddz_gi);
    self.Robot:Print("=======>  RobotGameDdz:cbDdzGameInfo  UID:"..self.Robot.Data.UID);
    
    if self:GetFsmState()=="TCreatePrvRoom" then
        
        --if self:GetRobotState(enum.STATE_DDZ_ROOM_OWNER) then
            -- �������Լ������ģ����ص��Ǵ����ɹ�
        --    self.IsCreate = true;
        --end
        
        
        self.Robot:Print("=========>Create private room.   UID:"..self.Robot.Data.UID);
        -- ��room_id���뵽�����б��У������������˼��롣
        
        local pb_room_info = PublicRoomInfo:new();
        pb_room_info.RoomType   = ddz_gi.private_room.room_type;
        pb_room_info.RoomID     = ddz_gi.room_id;
        pb_room_info.RoomRobots = ddz_gi.player_list;
        
        PublicRoomInfoMgr:PushOpenRoom(pb_room_info);
        
        self.Robot.GameFsm:SwitchState("TInRoomIdlem");
    elseif self:GetFsmState()=="TJoinPrvRoom" then
        self.Robot:Print("=========>Join private room.   UID:"..self.Robot.Data.UID);
        self.Robot.GameFsm:SwitchState("TInRoomIdlem");
    else
        
        self.Robot:Print("=========>Refresh private room.   UID:"..self.Robot.Data.UID);
    end
end

-- ����ҵ����׼����������Լ��͸���״̬
function RobotGameDdz:cbDdzUserStartReady( msgin )
    
    local ready_uid = msgin:rpb("PB.MsgInt");
    
    if self.Robot.Data.UID==ready_uid then
        
        self.SelfData.state = Misc.SetBit( self.SelfData.state, enum.STATE_DDZ_READY );
        self.Robot:Print("cbDdzUserStartReady");
    end
end

function RobotGameDdz:cbDDZ_QDZ_QX( msg_qdz )
    self.Robot:Print("SELF:"..self.Robot.Data.UID.."    cbDDZ_QDZ_QX  UID:"..msg_qdz.playid.."  WIK:"..msg_qdz.qingdizhu_wiki);
    
    if self.Robot.Data.UID==msg_qdz.playid then
        
        if msg_qdz.qingdizhu_wiki > 0 then
            
            local select_wki_list = {};
            
            for i=1,10 do
                if Misc.GetBit( msg_qdz.qingdizhu_wiki, i ) then
                    table.insert(select_wki_list, i);
                end
            end
            
            if #select_wki_list==0 then
                nlwarning("#select_wki_list==0");
                return;
            end
            
            local rnd           = math.random(#select_wki_list);
            local select_wki    = select_wki_list[rnd];
            
            self.Robot:Print("Select WKI:"..select_wki);
            local MsgQiangDiZhuResult = { result=select_wki };
            self.Robot:Send( "DDZ_QDZ", "PB.MsgQiangDiZhuResult", MsgQiangDiZhuResult );
        end
    end
end

function RobotGameDdz:cbDDZ_QDZ_F( msg_qdz_res )

    self.Robot:PrintTable(msg_qdz_res);
    
    local MsgJiaBeiResult = {
        result = math.random(2)
    };

    self.Robot:Send( "DDZ_JB", "PB.MsgJiaBeiResult", MsgJiaBeiResult );
end




return RobotGameDdz;
