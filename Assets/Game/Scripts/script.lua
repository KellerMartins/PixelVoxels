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

timers = {}
function float(entity)

    if timers[entity] == nil then
        timers[entity] = timer()
    end

    time = timers[entity]();

    newHeight = math.sin(time);
    pos = GetPosition(entity)
    SetPosition(entity, {x=pos.x, y=pos.y, z=pos.z+newHeight})
    rot = GetRotation(entity)
    SetRotation(entity, {x=rot.x, y=rot.y, z=time*20})
end

return "float"