module SpyduinoServer
  module Utils
    class Trilaterator
      attr_accessor

      def initialize(p1, p2, p3)
        @ex = (p2 - p1)/((p2 - p1).norm)
        @i = @ex.dot(p3-p1)
        @ey = (p3 - p1 - @i*@ex)/((p3 - p1 - @i*@ex).norm)
        @d = (p2 - p1).norm
        @j = @ey.dot(p3 - p1)
      end

      def calculate_from_distances dist1, dist2, dist3
        x = ((dist1**2) - (dist2**2) + (@d**2))/(2*@d)
        y = (((dist1**2) - (dist3**2) + (@i**2) + (@j**2))/(2*@j)) - ((@i/@j)*x)

        Vector[x, y]
      end

      def get_relative_point relative_point, origin_point
        Vector[relative_point[0] - origin_point[0], relative_point[1] - origin_point[1]]
      end
    end
  end
end
