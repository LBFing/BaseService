local BasePath = Misc.GetBasePath() .. "/script/";
package.path = BasePath .. "_PLS/?.lua;" .. BasePath .. "Framework/?.lua;";


require("InitFramework")


nlinfo("-=======DBSubStart==========-");


DBSubProc = require("DBSubProc");
sub_proc = DBSubProc:new();


--DBProc = require("DBProc");
--db_proc = DBProc:new();
--db_proc:LuaTest();




