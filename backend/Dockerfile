# Build
FROM eclipse-temurin:17-jdk-jammy AS builder
WORKDIR /app
COPY gradlew settings.gradle.kts build.gradle.kts gradle.properties ./
COPY gradle ./gradle
COPY src ./src
RUN chmod +x gradlew \
    && ./gradlew bootJar --no-daemon -x test \
    && JAR=$(find build/libs -maxdepth 1 -name "*.jar" ! -name "*-plain.jar" | head -1) \
    && cp "$JAR" /app/application.jar

# Run
FROM eclipse-temurin:17-jre-jammy
WORKDIR /app
COPY --from=builder /app/application.jar .
EXPOSE 8080
ENTRYPOINT ["java", "-jar", "/app/application.jar"]
