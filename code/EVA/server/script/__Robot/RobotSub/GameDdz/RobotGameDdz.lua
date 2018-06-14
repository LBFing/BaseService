local RobotGameDdz = class("RobotGameDdz", RobotGameBase)

function RobotGameDdz:ctor()
    self.super:ctor();
    self.Robot = nil;
    self.RoomInfo   = nil;
    self.IsCreate   = false;
end

function RobotGameDdz:GetFsmState()
    return self.Robot.GameFsm:GetState();
end

function RobotGameDdz:DoCreatePrvRoom()
    local create_prvroom = { room_type="RM_DDZ" };
    self.Robot:Send( "CPRM", "PB.MsgCreatePrivateRoom", create_prvroom );
end

function RobotGameDdz:DoJoinPrvRoom(open_room)
    local join_prvroom = { room_id=open_room.RoomID, room_type=open_room.RoomType };
    self.Robot:Send( "EPRM", "PB.MsgEnterPrivateRoom", join_prvroom );
end




function RobotGameDdz:cbDdzGameInfo( msgin )
    
    local ddz_gi = msgin:rpb("PB.MsgDDZRoom");
    
    if ddz_gi==nil then
        nlwarning("ddz_gi==nil !!!!!!!!!!!!");
        return
    end

    self.RoomInfo = ddz_gi;
    
    
    PrintTable(ddz_gi);
    nlinfo("=======>  RobotGameDdz:cbDdzGameInfo  UID:"..self.Robot.Data.UID);
    
    if self:GetFsmState()=="TCreatePrvRoom" then

        
        for _,v in ipairs(ddz_gi.player_list) do
            if v.player_base.UID == self.Robot.Data.UID then
                if Misc.GetBit(v.state, enum.STATE_DDZ_ROOM_OWNER) then
                    -- �������Լ������ģ����ص��Ǵ����ɹ�
                    self.IsCreate = true;
                end
            end
        end
        
        nlinfo("=========>Create private room.   UID:"..self.Robot.Data.UID);
        -- ��room_id���뵽�����б��У������������˼��롣
        
        local pb_room_info = PublicRoomInfo:new();
        pb_room_info.RoomType   = ddz_gi.private_room.room_type;
        pb_room_info.RoomID     = ddz_gi.room_id;
        pb_room_info.RoomRobots = ddz_gi.player_list;
        
        PublicRoomInfoMgr:PushOpenRoom(pb_room_info);
        
        self.Robot.GameFsm:SwitchState("TIdle");
    else
        
        nlinfo("=========>Refresh private room.   UID:"..self.Robot.Data.UID);
    end
end


return RobotGameDdz;
