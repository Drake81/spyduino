module SpyduinoServer
  module Utils
    class ProtocolParser
      def initialize(client)
        @client = client
        @state = :waiting_for_begin
        @data = []
      end

      def get_package
        loop do
          break if [:completed, :parse_error].include?(@state)
          read_line
        end

        tmp = @state == :completed ? @data :  []
        reset

        tmp
      end

      def read_line
        line = @client.gets

        transition_to(:parse_error) and return(nil) unless line

        line.chomp!

        if @state == :waiting_for_begin && line == "AT+CWLAP"
          transition_to :reading_data

        elsif @state == :reading_data && (line =~ /^\+CWLAP\:/ || line == "")
          if line == ""
            transition_to :search_ok
          else
            parse_data line
          end

        elsif @state == :search_ok && line == "OK"
          transition_to :completed

        else
          transition_to :parse_error
          puts "PARSE ERROR"
        end
      end

      private

      def transition_to state
        @state = state
      end

      def parse_data line
        match = line.match(/^\+CWLAP:\(\d,\"(?<ssid>.+)\",-(?<signal_strength>\d+),\"(?<mac>.+)\",\d+\)$/)
        if match
          @data << {
            ssid: match[:ssid],
            signal_strength: match[:signal_strength].to_i,
            mac: match[:mac]
          }
        end
      end

      def reset
        @data = []
        @state = :waiting_for_begin
      end
    end
  end
end
