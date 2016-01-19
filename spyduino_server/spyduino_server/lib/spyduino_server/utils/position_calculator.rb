require 'matrix'

module SpyduinoServer
  module Utils
    class PositionCalculator
      def get_position(data)
        data.select! {|d| d[:signal_strength] < 85}
        access_point_distances = extract_access_points(data)

        return nil if access_point_distances.count <=2

        calculate_position(access_point_distances)
      end


      private
      def distance(signal_strength)
        10**(    (     (signal_strength-10) + 147.55 -  (20*Math.log10(2.4*(10**9)))     )    /20.0      )
      end

      def calculate_position(ap_distances)
        positions = []
        (0..ap_distances.count-3).each do |i|
          (i+1..ap_distances.count-3).each do |j|
            aps = ([i] + [j,j+1]).collect { |k| ap_distances[k][0].position }
            distances = ([i] + [j,j+1]).collect { |k| ap_distances[k][1] }
            trilaterator = Trilaterator.new(*aps)

            positions << trilaterator.calculate_from_distances(*distances)
          end
        end

        positions.select! do |pos|
          (0..50).include?(pos[0]) && (0..20).include?(pos[1])
        end

        return nil unless positions.any?

        x = positions.inject(0) {|sum,pos| sum + pos[0]} / positions.count
        y = positions.inject(0) {|sum,pos| sum + pos[1]} / positions.count

        Position.new(x,y)
      end

      def extract_access_points(data)
        access_point_distances = data.inject [] do |list,package|
          ap = AccessPoint.find_by_mac(package[:mac])
          list << [ap,distance(package[:signal_strength])] if ap
          list
        end

        access_point_distances || []
      end
    end
  end
end
