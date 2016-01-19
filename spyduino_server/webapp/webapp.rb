#!/usr/bin/env ruby

require 'rubygems'
require 'bundler/setup'#!/usr/bin/env ruby

require 'spyduino_server'

require 'sinatra/base'
require 'haml'

class Webapp < Sinatra::Application
  get "/" do
    @floorplan = params[:floorplan] || 'uni'
    @scaleFactor = get_scale_factor
    @positions = SpyduinoServer::Utils::Position.all
    haml :floorplan
  end

  def get_scale_factor
    case @floorplan
      when 'uni' then 970.1559787326389
      when 'wohnung' then 1729.7211740912142
    end
  end
end
