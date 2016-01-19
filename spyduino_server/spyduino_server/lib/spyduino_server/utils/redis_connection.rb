require 'redis'
require 'connection_pool'

module SpyduinoServer
  module Utils
    module RedisConnection
      def self.with &block
        connection_pool.with do |conn|
          block.call(conn)
        end
      end

      def self.connection_pool
        @connection_pool ||= ConnectionPool.new(size: 5, timeout: 5) {Redis.new}
      end
    end
  end
end
