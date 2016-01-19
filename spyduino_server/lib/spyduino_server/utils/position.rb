require 'matrix'
require 'date'

module SpyduinoServer
  module Utils
    class Position
      attr_reader :timestamp, :x, :y


      def self.last
        data = nil
        RedisConnection.with do |conn|
          data = conn.lindex "positions", "0"
        end

        return nil unless data

        timestamp,x,y = data.split("|")

        new(x.to_f,y.to_f ,DateTime.parse(timestamp))
      end

      def self.all
        datas = nil
        RedisConnection.with do |conn|
          count = conn.llen "positions"
          datas = conn.lrange "positions", 0, count-1
        end

        #return nil unless data
        datas.map do |data|
          timestamp,x,y = data.split("|")
          new(x.to_f,y.to_f ,DateTime.parse(timestamp))
        end
      end

      def initialize(x, y, timestamp = DateTime.now)
        @timestamp = timestamp
        @x = x
        @y = y
      end

      def save
        RedisConnection.with do |conn|
          conn.lpush "positions", [timestamp.to_s, x, y].join("|")
        end
      end

      def vector
        Vector[x,y]
      end
    end
  end
end
