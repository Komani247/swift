module Swift
  class Scheme
    def self.migrations &migrations
      define_singleton_method(:migrate!, lambda{|db = Swift.db| migrations.call(db)})
    end

    def self.migrate! db = Swift.db
      db.migrate! self
    end
  end # Scheme

  def self.migrate! name = nil
    schema.each{|scheme| scheme.migrate!(db(name)) }
  end
end # Swift
