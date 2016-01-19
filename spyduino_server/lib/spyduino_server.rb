require 'rubygems'
require 'bundler/setup'

require 'spyduino_server/utils'

module SpyduinoServer
  def self.redis
    Utils::RedisConnection.redis
  end
end
