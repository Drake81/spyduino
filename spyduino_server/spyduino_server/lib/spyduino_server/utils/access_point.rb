require 'matrix'

module SpyduinoServer
  module Utils
    class AccessPoint
      attr_reader :ssid, :mac, :position


      def self.all
        RedisConnection.with do |conn|
          conn.keys("ap:*").inject([]) do |list,ap_key|
            data = conn.hgetall(ap_key)
            list << new(data["ssid"],data["mac"],Vector[data["x"].to_f,data["y"].to_f])
            list
          end
        end
      end

      def self.find_by_mac(mac)
        data = nil

        RedisConnection.with do |conn|
          data = conn.hgetall("ap:#{mac}")
        end

        data.empty? ? nil : new(data["ssid"],data["mac"],Vector[data["x"].to_f,data["y"].to_f])
      end

      def initialize(ssid,mac,position)
        @ssid = ssid
        @mac = mac
        @position = position
      end

      def id
        "ap:#{mac}"
      end

      def save
        RedisConnection.with do |conn|
          conn.mapped_hmset id, {ssid: ssid, mac: mac, x: position[0], y: position[1]}
        end
      end
    end
  end
end
