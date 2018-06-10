Map = class("Map")

local this = Map

function this:ctor()
	self.map = {};
    self.count = 0
end

-- Map 插入新值
function this:Insert(k,v)
    if nil == self.map[k] then
        self.map[k] = v
        self.count = self.count + 1
    end
end

-- Map 插入新值并且切换旧值
function this:Replace(k,v)
	if nil == self.map[k] then
		self.map[k] = v;
		self.count = self.count + 1;
	else
		self.map[k] = v;
	end
end

function this:Remove(k)
    if nil ~= self.map[k] then
        self.map[k] = nil
        if self.count >0 then
            self.count = self.count - 1
        end
    end
end

function this:ForEachRemove(field, value)

    local newT = {} 
    
	for k,v in pairs(self.map) do
        if v[field]~=value then
            newT[k] = v;
        end
    end 
    
    self.map = newT;
end

function this:Find(k)
    return self.map[k]
end

function this:Clear()
    for k,_ in pairs(self.map) do
        if nil ~= self.map[k] then
            self.map[k] = nil
        end
    end
    self.count = 0
end


-- 遍历所有成员
function this:ForEach(fun, ...)
	-- body
	for k,v in pairs(self.map) do
		fun(k, v, ...)
	end
end

-- Map 获取字典的count
function this:Count()
	return self.count;
end

return Map;


--local characters = Map:new()
--characters:Insert("name1"," this Name:123")
--characters:Replace("name1"," this Name:2" )
--local name2 = characters:Find("name1")
--nlinfo(name2)
--nlinfo(characters.count)
--for k,v in pairs(characters) do
--nlinfo(k,v)
--end