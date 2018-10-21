-- script.lua

function timer()
    local timer = 0;

    return function()
            timer = timer + 0.1
            return timer;
        end
end

function getTable(foo)
    print(foo)
    
    io.write("The table the script received has:\n");
    for k, v in pairs(foo) do
        print(k, v)
    end
end

LoadFontTTF("Interface/Fonts","Visitor",64)
LoadFontTTF("Interface/Fonts","Visitor",128)
LoadFontTTF("Interface/Fonts/coolthre","CoolThre",16)
LoadFontTTF("Interface/Fonts/gros/","Gros",16)

camPos = {x=0, y=0, z = 0}

timers = {}
function float(entity)

    if timers[entity] == nil then
        timers[entity] = timer()
    end

    time = timers[entity]();


    DrawRectangle({x = 200,y = 200}, {x = 300,y = 300}, {r = 0.25,g = 0.25,b = 0.25})
    DrawTextColored("PARTY!", {r = math.abs(math.sin(time)),g = math.abs(math.sin(time*2)),b = math.abs(math.sin(time*3))}, PositionToGameScreenCoords(GetPosition(entity)), "Gros16")
 
    for i=0,10,0.5 do
        t = i+time
        DrawLine({x = (i)*10+200,y = 500+math.sin(t/2)*50}, {x = (i+0.5)*10+200,y = 500+math.sin((t+0.5)/2)*50}, 5 ,{g = (math.sin(t/2)+1)/2,r = 0.0,b = math.abs(math.sin(time/2))})
    end

    newHeight = math.sin(time);
    pos = GetPosition(entity)
    SetPosition(entity, {x=pos.x, y=pos.y, z=pos.z+newHeight})
    rot = GetRotation(entity)
    SetRotation(entity, {x=time*20, y=time*20, z=time*20})

    if GetKey("UP") then
        camPos.y = 100;
    elseif GetKey("DOWN") then
        camPos.y = -100;
        else
        camPos.y = 0;
        end

    if GetKey("LEFT") then
        camPos.x = -100;
    elseif GetKey("RIGHT") then
        camPos.x = 100;
    else 
        camPos.x = 0;
    end

    MoveCamera(camPos)

end

return "float"