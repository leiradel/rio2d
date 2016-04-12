' Public domain

sub growBomb(target as node, clickPos as vec2)
  target.x = clickPos.x
  target.y = clickPos.y
  target.scale = 0.1
  target.visible = true
  target.opacity = 50

  parallel
    sequence
      target.scaleTo 1 in 0.06 secs
    end
    
    sequence
      pause 2 secs
      target.opacity = 255
    end
  end
end

sub rot(target as node)
  forever
    target.rotateBy -90 in 0.5 secs
  end
end

sub swingHealth(target as node, screenSize as size, speed as number)
  source_x = rand(0, floor(screenSize.width * 0.8)) + screenSize.width * 0.1
  target_x = rand(0, floor(screenSize.width * 0.8)) + screenSize.width * 0.1
  
  target.x = source_x
  target.y = screenSize.height + target.bboxHeight * 0.5
  target.visible = true
  
  parallel
    forever
      target.rotateTo -10 in 1.2 secs with quadinout
      target.rotateTo 10 in 1.2 secs with quadinout
    end
    
    sequence
      target.moveTo target_x, screenSize.height * 0.15 in speed secs
      signal "fallingDone"
    end
  end
end

sub meteorFall(target as node, screenSize as size, speed as number)
	meteor_x = rand(0, floor(screenSize.width * 0.8)) + screenSize.width * 0.1
	target_x = rand(0, floor(screenSize.width * 0.8)) + screenSize.width * 0.1
  
	target.x = meteor_x
  target.y = screenSize.height + target.bboxHeight * 0.5
  target.visible = true
  
  if rand(0, 1) = 0 then
    dir = -90
  else
    dir = 90
  end
  
  parallel
    forever
      target.rotateBy dir in 0.5 secs
    end
    
    sequence
      target.moveTo target_x, screenSize.height * 0.15 in speed secs
      signal "fallingDone"
    end
  end
end

sub shockWave(target as node, bomb as node)
  target.scale = 0.1
  target.x = bomb.x
  target.y = bomb.y
  target.visible = true
  target.opacity = 255
  
  targetScale = bomb.scale * 2
  
  parallel
    sequence
      target.scaleTo targetScale in 0.5 secs
    end
    
    sequence
      target.fadeOut in 1 secs
      target.visible = false
    end
  end
end

sub groundHit(target as node, screenSize as size, frms as frames)
  target.moveby 0, screenSize.height * 0.12 in 0 secs
  
  for i = 1 to frms.length - 1
    target.setFrame frms, i
    pause 0.1 secs
  next

  ' frms[0] contains the meteor sprite frame, so we reset the sprite to it
  target.setFrame frms, 0
  target.visible = false
end

sub explosion(target as node, frms as frames)
  for i = 0 to frms.length - 1
    target.setFrame frms, i
    pause 0.0714285714285714 secs
  next
  
  target.visible = false
end
