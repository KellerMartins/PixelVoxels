-- script.lua

function timer()
    local timer = 0;

    return function()
            timer = timer + 0.1
            return timer;
        end
end

function getTable()

    local foo = GetPosition(0)

    print(foo)
    
    io.write("The table the script received has:\n");
    x = 0
    for k, v in pairs(foo) do
        print(k, v)
        x = x + v
    end
    io.write("Returning data back to C\n");
    return x
end

floatTimer = timer();
function float(entity)
    time = floatTimer();

    newHeight = math.sin(time)*10 + 20;
    pos = GetPosition(entity)
    SetPosition(entity, {x=pos.x, y=pos.y, z=newHeight})
    rot = GetRotation(entity)
    SetRotation(entity, {x=rot.x, y=rot.y, z=time*20})
end

return "float"