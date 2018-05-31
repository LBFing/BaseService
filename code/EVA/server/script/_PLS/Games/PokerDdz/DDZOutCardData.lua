local DDZOutCardData = class("DDZOutCardData")

-- 构造函数;
function DDZOutCardData:ctor()
    self:ClearData();
end

function DDZOutCardData:ClearData()
    self.UID            = 0;
    self.Type           = 0;
    self.Cards          = {};
end

function DDZOutCardData:IsEmpty()
    if self.UID==0 or self.Type==0 then
        return true;
    end
    return false;
end


return DDZOutCardData;
