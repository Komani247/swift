require_relative 'helper'

describe 'Adapter' do
   supported_by Swift::DB::Postgres, Swift::DB::Mysql do
    describe 'Storing binary objects' do
      before do
        Swift.db do |db|
          type = db.is_a?(Swift::DB::Postgres) ? 'bytea' : 'blob'
          db.execute %q{drop table if exists users}
          db.execute %Q{create table users(id serial, name text, image #{type}, primary key(id))}
        end
      end
      it 'should store and retrieve image' do
        Swift.db do |db|
          data = Swift::BLOB.new(File.read(File.dirname(__FILE__) + '/house-explode.jpg'))
          db.prepare("insert into users (name, image) values(?, ?)").execute('test', data)
          value = db.prepare("select image from users limit 1").execute.first[:image]
          assert_equal Encoding::ASCII_8BIT, value.encoding
          assert_equal data.force_encoding("ASCII-8BIT"), value
        end
      end
    end
   end
end
