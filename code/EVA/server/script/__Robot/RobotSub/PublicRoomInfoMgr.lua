PublicRoomInfoMgr = {}

-- ������Ϣ��robot�佻����Ϣʹ�á�
function PublicRoomInfoMgr:Init()
	
    self.Rooms      =   {};  -- room_prvid  ,  room_ins

end


function PublicRoomInfoMgr:PushOpenRoom( room_info )
    self.Rooms[room_info.RoomID] = room_info;
end

function PublicRoomInfoMgr:RemoveOpenRoom( room_prvid )
	self.Rooms[room_prvid] = nil;
end

function PublicRoomInfoMgr:GetOpenRoom()
    for _,v in pairs(self.Rooms) do
        return v;
    end
    return nil;
end

return PublicRoomInfoMgr;
